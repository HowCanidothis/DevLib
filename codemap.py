#!/usr/bin/env python3
"""codemap.py - builds a compact code index for Claude (no dependencies, Python 3.8+).

Run from the project root.
  python codemap.py index [FOLDER] [--out DIR] [--part-chars 150000] [--private]
  python codemap.py dump FILE [FILE ...] [--out .codemap/_dump]
  python codemap.py section FOLDER FILE.md      (replace '## FOLDER' in CODEMAP.md under a lock)
  python codemap.py section FOLDER --show       (print the current section)
  python codemap.py section NAME --show --file CONVENTIONS.md   (any '## NAME' section of another file)
  python codemap.py find NAME                   (where NAME is declared / implemented / used)
  python codemap.py status                      (OK/OLD/CHANGED/MISSING per indexed folder, by content hash)
  python codemap.py reindex [--all]             (rebuild OLD/CHANGED indexes; free, no AI)

Several projects (e.g. an app and a library submodule) keep separate maps that work as one:
codemap.conf in the project root (commit it):
  name = wps-client              # map title "# Карта кода <name>" (default: folder name)
  links = libs/DevLib            # other codemap projects: find/status/reindex cover them too
  exclude = 3rdparty thirdparty  # extra folders to skip (names or paths from the project root)
A subfolder that has its own codemap.py / codemap.conf is a separate project and is never
indexed into this map; link it instead.

index libs/DevLib/PropertiesModule -> .codemap/libs__DevLib__PropertiesModule/
      00_overview.md (tree, stats, build files) + index_NN.md (per-file skeletons)
      Each folder gets its own subfolder, so folders can be indexed one at a time.
dump  -> .codemap/_dump/dump_NN.md (full text of the requested files)
"""
import argparse, ast, bisect, os, re, sys
from collections import defaultdict

SKIP_DIRS = {'.git', '.svn', '.hg', '.vs', '.vscode', '.idea', 'node_modules', '__pycache__',
             'venv', '.venv', 'env', 'dist', 'build', 'out', 'bin', 'obj', 'Debug', 'Release',
             'x64', 'x86', '.codemap', 'GeneratedFiles', '.cache', 'coverage'}
CPP_EXT = {'.h', '.hh', '.hpp', '.hxx', '.inl', '.c', '.cc', '.cpp', '.cxx', '.ipp'}
HDR_EXT = {'.h', '.hh', '.hpp', '.hxx', '.inl', '.ipp'}
JS_EXT = {'.js', '.jsx', '.ts', '.tsx', '.mjs', '.cjs'}
OTHER_EXT = {'.cs', '.java', '.go', '.rs', '.kt', '.swift', '.qml'}
BUILD_NAMES = {'CMakeLists.txt', 'package.json', 'pyproject.toml', 'setup.py', 'Cargo.toml', 'go.mod'}
BUILD_EXT = {'.pro', '.pri', '.cmake', '.qbs', '.sln', '.vcxproj'}
MAX_FILE = 2_000_000
CODEMAP_VERSION = 5
USES_LIMIT = 40


def skip_dir(name):
    return name in SKIP_DIRS or name.startswith('cmake-build') or name.startswith('build-')


def read(path):
    with open(path, 'rb') as f:
        data = f.read(MAX_FILE)
    for enc in ('utf-8', 'cp1251', 'latin-1'):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            pass
    return data.decode('utf-8', 'replace')


def ws(s, limit=180):
    s = re.sub(r'\s+', ' ', s).strip()
    s = re.sub(r'\s*([,)])', r'\1', s)
    s = re.sub(r'([(<])\s+', r'\1', s)
    s = re.sub(r'\s+([(>])', r'\1', s)
    s = re.sub(r',(?=\S)', ', ', s)
    return s if len(s) <= limit else s[:limit - 3] + '...'


def first_comment(text, hash_comments=False):
    text = re.sub(r'^\s*#\s*(ifndef|define|pragma)[^\n]*\n', '', text, count=2, flags=re.M) if not hash_comments else text
    pat = r'\s*(?:(?://[^\n]*\n\s*)+|/\*.*?\*/' + (r'|#[^\n]*\n' if hash_comments else '') + r'|"""(.*?)"""|\'\'\'(.*?)\'\'\')'
    m = re.match(pat, text, re.S)
    if not m:
        return ''
    c = m.group(0)
    lines = [re.sub(r'^\s*(?://+|/\*+|\*+/?|#+|"""|\'\'\')\s*', '', l).strip() for l in c.splitlines()]
    lines = [l for l in lines if l and not re.match(r'(?i)(copyright|license|\(c\)|-\*-|!/usr|pragma|include|coding)', l)
             and not re.fullmatch(r'[*=/#-]+', l)]
    return ws(' '.join(lines[:3]), 200)


# ---------------------------------------------------------------- C / C++
CPP_TOKENS = re.compile(r'//[^\n]*|/\*.*?\*/|R"(\w*)\(.*?\)\1"|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)
ACCESS = re.compile(r'^\s*(public|protected|private|signals|Q_SIGNALS|public\s+slots|protected\s+slots|private\s+slots|'
                    r'public\s+Q_SLOTS|protected\s+Q_SLOTS|private\s+Q_SLOTS|slots|Q_SLOTS)\s*:(?!:)')
MACRO = re.compile(r'^\s*([A-Z][A-Z0-9_]{2,})\b\s*(\((?:[^()]|\([^()]*\))*\))?')
INIT_LIST = re.compile(r'\)\s*(?:const\s*|noexcept\s*|override\s*|final\s*)*:(?!:)')
SKIP_START = re.compile(r'^\s*(typedef|using|return|friend|static_assert|template\s*<[^>]*>\s*friend|delete|throw|goto)\b')


def cpp_clean(text):
    def rep(m):
        s = m.group(0)
        if s.startswith('/'):
            return '\n' * s.count('\n') or ' '
        return '""' + '\n' * s.count('\n')
    return CPP_TOKENS.sub(rep, text)


IGNORED_CONDS = {'__cplusplus', 'defined __cplusplus', 'defined(__cplusplus)'}


def pp_conditions(src):
    """For every line of comment-free source: the active #if conditions as a string ('' = unconditional).
    Header guards and __cplusplus are ignored. Also returns non-guard #define names."""
    lines = src.split('\n')
    conds, stack, defines = [], [], []
    guard = None
    directives = [(i, l.strip()) for i, l in enumerate(lines) if l.lstrip().startswith('#')]
    if len(directives) >= 2:
        m1 = re.match(r'#\s*ifndef\s+(\w+)', directives[0][1])
        m2 = re.match(r'#\s*define\s+(\w+)\s*$', directives[1][1])
        if m1 and m2 and m1.group(1) == m2.group(1):
            guard = m1.group(1)
    def norm(e):
        e = re.sub(r'\s+', ' ', e).strip()
        e = re.sub(r'defined\s*\(\s*(\w+)\s*\)', r'\1', e)
        e = re.sub(r'defined\s+(\w+)', r'\1', e)
        return e
    def neg(e):
        return e[1:] if e.startswith('!') and re.fullmatch(r'!\w+', e) else ('!' + e if re.fullmatch(r'\w+', e) else f'!({e})')
    cont = ''
    for i, l in enumerate(lines):
        t = (cont + l).strip()
        if t.endswith('\\'):
            cont = t[:-1] + ' '
        else:
            cont = ''
            m = re.match(r'#\s*(ifdef|ifndef|if|elif|else|endif|define)\b\s*(.*)', t)
            if m:
                d, arg = m.group(1), m.group(2).strip()
                if d == 'ifdef':
                    stack.append(norm(arg.split()[0]) if arg else '?')
                elif d == 'ifndef':
                    name = arg.split()[0] if arg else '?'
                    stack.append(None if name == guard else '!' + name)
                elif d == 'if':
                    stack.append(norm(arg))
                elif d == 'elif' and stack:
                    prev = stack[-1]
                    stack[-1] = norm(arg) if prev is None else f'{neg(prev)} && {norm(arg)}' if '&&' not in prev else norm(arg)
                elif d == 'else' and stack:
                    stack[-1] = None if stack[-1] is None else neg(stack[-1])
                elif d == 'endif' and stack:
                    stack.pop()
                elif d == 'define':
                    mm = re.match(r'(\w+)(\()?', arg)
                    if mm and mm.group(1) != guard:
                        defines.append(mm.group(1) + ('()' if mm.group(2) else ''))
        active = [c for c in stack if c and c not in IGNORED_CONDS]
        conds.append(' && '.join(active))
    return conds, defines



def func_name(stmt):
    """Return name before the first top-level '(' if stmt looks like a function, else None."""
    depth = 0
    for i, ch in enumerate(stmt):
        if ch == '<':
            depth += 1
        elif ch == '>':
            depth = max(0, depth - 1)
        elif ch == '=' and depth == 0 and 'operator' not in stmt[:i + 2]:
            return None
        elif ch == '(' and depth == 0:
            m = re.search(r'(operator\s*[^\s(]+|operator\s*\(\s*\)|[~\w:]+)\s*$', stmt[:i])
            if not m:
                return None
            name = m.group(1)
            if name in ('if', 'for', 'while', 'switch', 'catch', 'sizeof', 'decltype', 'alignof') or name.isupper():
                return None
            return name
    return None


def parse_cpp(text, is_header, show_private):
    includes = []
    src = cpp_clean(text)
    line_cond, defines = pp_conditions(src)
    src_lines = src.split('\n')
    for m in re.finditer(r'^[ \t]*#[ \t]*include[ \t]*([<"][^>"\n]+[>"])', text, re.M):
        ln = text.count('\n', 0, m.start())
        if ln >= len(src_lines) or not src_lines[ln].lstrip().startswith('#'):
            continue  # commented out
        c = line_cond[ln]
        includes.append(m.group(1) + (f'[if {c}]' if c else ''))
    src = re.sub(r'^[ \t]*#(?:[^\n]*\\\n)*[^\n]*', lambda m: '\n' * m.group(0).count('\n'), src, flags=re.M)  # drop preprocessor
    nl = [k for k, ch in enumerate(src) if ch == '\n']
    cur = [0]

    def cond_tag():
        c = line_cond[bisect.bisect_left(nl, cur[0])] if line_cond else ''
        return f' [if {c}]' if c else ''

    class _Out(list):
        def append(self, item):
            list.append(self, item + cond_tag())
    out = _Out()
    defs = defaultdict(list)  # cpp: class -> methods
    stack = [{'kind': 'ns', 'access': 'public', 'indent': 0}]
    stmt, paren = '', 0

    def indent():
        return '  ' * (sum(1 for s in stack if s['kind'] in ('class', 'ns') and s.get('named')))

    def strip_macros(s, scope):
        while True:
            m = ACCESS.match(s)
            if m:
                acc = re.sub(r'\s+', ' ', m.group(1)).replace('Q_SIGNALS', 'signals').replace('Q_SLOTS', 'slots')
                scope['access'] = acc
                s = s[m.end():]
                continue
            m = MACRO.match(s)
            if m and (m.group(2) or not re.match(r'\s*[\w:<]', s[m.end():] or 'x') or m.group(1).startswith('Q_')):
                name, args = m.group(1), m.group(2) or ''
                if name == 'Q_PROPERTY' and is_header:
                    out.append(indent() + 'prop ' + ws(args[1:-1], 140))
                elif name in ('Q_OBJECT', 'Q_GADGET') and scope['kind'] == 'class':
                    scope['qobj'] = name
                s = s[m.end():]
                continue
            return s

    def record_decl(s, scope, definition):
        s = ws(s)
        name = func_name(s)
        if not name or SKIP_START.match(s):
            return
        acc = scope.get('access', 'public')
        if scope['kind'] == 'class' and acc.startswith('private') and 'slots' not in acc and not show_private:
            return
        if is_header or scope['kind'] == 'class':
            tag = {'signals': 'sig ', 'public slots': 'slot ', 'protected slots': 'slot# ', 'private slots': 'slot- ',
                   'slots': 'slot ', 'protected': '# ', 'private': '- '}.get(acc, '') if scope['kind'] == 'class' else 'fn '
            sig = re.sub(r'\s*(override|Q_DECL_OVERRIDE)\b', '', s)
            m = INIT_LIST.search(sig)
            if m:
                sig = sig[:m.end() - 1].rstrip()
            out.append(indent() + tag + sig)
        elif definition:
            if '::' in name:
                cls, meth = name.rsplit('::', 1)
                defs[cls].append(meth + cond_tag().replace(' [', '['))
            else:
                defs[''].append(ws(s, 120) + cond_tag())

    def record_alias(s, scope):
        """using X = ...; / typedef ... X; in headers -> 'alias X = ...'."""
        if not is_header or scope['kind'] not in ('ns', 'class'):
            return False
        if scope['kind'] == 'class' and scope.get('access', 'public').startswith('private') and not show_private:
            return False
        t = ws(s, 400)
        m = re.match(r'(?:template\s*<.*?>\s*)?using\s+(\w+)\s*=\s*(.+)$', t)
        if m:
            out.append(indent() + f'alias {m.group(1)} = {ws(m.group(2), 140)}')
            return True
        m = re.match(r'typedef\s+(.+?)\s*\b(\w+)\s*(\[[^\]]*\])?$', t)
        if m and '(' not in t:
            out.append(indent() + f'alias {m.group(2)} = {ws(m.group(1), 140)}')
            return True
        return t.startswith(('using ', 'typedef '))

    i, n = 0, len(src)
    while i < n:
        cur[0] = i
        ch = src[i]
        scope = stack[-1]
        if ch == '(':
            paren += 1
        elif ch == ')':
            paren = max(0, paren - 1)
        if scope['kind'] in ('body', 'skip') and ch not in '{}':
            i += 1
            continue
        if ch == '{' and paren == 0:
            if scope['kind'] in ('body', 'skip'):
                stack.append({'kind': 'skip'})
                i += 1
                continue
            s = strip_macros(stmt, scope)
            before = s.rstrip()
            if INIT_LIST.search(s) and before and (before[-1].isalnum() or before[-1] in '_>'):
                stack.append({'kind': 'init'})
                stmt += '{'
                i += 1
                continue
            m_ns = re.match(r'\s*(?:inline\s+)?namespace\b\s*([\w:]*)', s)
            m_ext = re.match(r'\s*extern\s*""', s)
            m_enum = re.search(r'\benum\b(?:\s+class|\s+struct)?\s*(\w*)\s*(:\s*[\w:]+)?\s*$', s)
            # drop UPPER_CASE macro calls (e.g. ATTACH_MEMORY_SPY_2(X), Q_DECL_EXPORT) from a class head
            s_head = re.sub(r'\b[A-Z][A-Z0-9_]{2,}\s*\((?:[^()]|\([^()]*\))*\)', ' ', s)
            m_cls = re.search(r'\b(class|struct|union)\b([^{]*)$', s_head)
            if m_ns or m_ext:
                name = m_ns.group(1) if m_ns else ''
                if name and is_header:
                    out.append(indent() + 'namespace ' + name)
                stack.append({'kind': 'ns', 'access': 'public', 'named': bool(name and is_header)})
            elif m_enum and '(' not in s:
                close = src.find('}', i)
                vals = re.findall(r'(\w+)\s*(?:=[^,}]*)?(?:,|$)', src[i + 1:close])
                vals = [v for v in vals if v]
                if is_header or scope['kind'] == 'class':
                    shown = ', '.join(vals[:12]) + (', ...' if len(vals) > 12 else '')
                    out.append(indent() + f'enum {m_enum.group(1) or "(anon)"} {{{shown}}}')
                stack.append({'kind': 'skip'})
            elif m_cls and '(' not in m_cls.group(2) and '=' not in s_head:
                head = m_cls.group(2)
                parts = re.split(r'(?<!:):(?!:)', head, maxsplit=1)
                words = [w for w in re.findall(r'[\w:]+', re.sub(r'<.*>', '', parts[0])) if w not in ('final', 'alignas')]
                name = words[-1] if words else '(anon)'
                bases = ws(parts[1], 120) if len(parts) > 1 else ''
                kind = m_cls.group(1)
                new = {'kind': 'class', 'access': 'private' if kind == 'class' else 'public', 'named': True, 'name': name}
                stack.append(new)
                out.append(indent()[:-2] + f'{kind} {name}' + (f' : {bases}' if bases else '') + '  @@QOBJ' + str(id(new)))
                new['line'] = len(out) - 1
            elif func_name(ws(s)) and not re.search(r'=\s*\[', s):
                record_decl(s, scope, True)
                stack.append({'kind': 'body'})
            else:
                stack.append({'kind': 'skip'})
            stmt = ''
        elif ch == '}' and paren == 0:
            if len(stack) > 1:
                top = stack.pop()
                if top['kind'] == 'init':
                    stmt += '}'
                    i += 1
                    continue
                if top['kind'] == 'class':
                    tag = top.get('qobj', '')
                    out[top['line']] = out[top['line']].replace('  @@QOBJ' + str(id(top)), f' [{tag}]' if tag else '')
            if stack[-1]['kind'] not in ('body', 'skip'):
                stmt = ''
        elif ch == ';' and paren == 0:
            s = strip_macros(stmt, scope)
            if not record_alias(s, scope) and '(' in s:
                record_decl(s, scope, False)
            stmt = ''
        else:
            stmt += ch
            if ch == ':' and paren == 0:
                s2 = strip_macros(stmt, scope)  # consume access labels eagerly
                if s2.strip() == '':
                    stmt = ''
        i += 1
    for line in out:
        if '@@QOBJ' in line:
            out[out.index(line)] = re.sub(r'  @@QOBJ\d+', '', line)
    if defs:
        for cls, meths in defs.items():
            if cls:
                uniq = list(dict.fromkeys(meths))
                list.append(out, f'impl {cls}: ' + ', '.join(uniq[:40]) + (f', ...(+{len(uniq) - 40})' if len(uniq) > 40 else ''))
        for f in defs.get('', [])[:40]:
            list.append(out, 'fn ' + f)
    # types this file uses but does not declare (fields, params, bodies) -> 'uses:' line
    declared = set(re.findall(r'\b(?:class|struct|union|enum(?:\s+class)?)\s+(?:[A-Z_][A-Z0-9_]*\s+)*(\w+)', src))
    declared |= set(re.findall(r'\busing\s+(\w+)\s*=', src))
    declared |= set(re.findall(r'\btypedef\b[^;]*?\b(\w+)\s*;', src))
    declared |= set(defs)
    counts = defaultdict(int)
    # a type is followed by a name, '<', '::', '*'/'&' or created with 'new'; not after '::', '.', '->'
    type_use = re.compile(r'(?<![\w.:>])(?:new\s+)?([A-Z][A-Za-z0-9_]{2,})\b(?=\s*(?:<|::|[*&]+|\s[*&]*\s*[A-Za-z_]))')
    for tok in type_use.findall(src):
        if tok.isupper() or re.match(r'Q[A-Z]|Qt[A-Z]?', tok) or tok in declared:
            continue
        counts[tok] += 1
    if defines:
        uniq = list(dict.fromkeys(defines))
        list.append(out, 'defines: ' + ' '.join(uniq[:30]) + (f' ...(+{len(uniq) - 30})' if len(uniq) > 30 else ''))
    if counts:
        top = sorted(counts, key=lambda k: (-counts[k], k))[:USES_LIMIT]
        list.append(out, 'uses: ' + ' '.join(sorted(top)) + (f' ...(+{len(counts) - USES_LIMIT})' if len(counts) > USES_LIMIT else ''))
    return includes, out


# ---------------------------------------------------------------- Python
def parse_py(text):
    try:
        tree = ast.parse(text)
    except SyntaxError:
        return parse_generic(text, '.py')
    imports, out = [], []
    doc = ast.get_docstring(tree)

    def sig(fn):
        a = fn.args
        names = [x.arg for x in a.posonlyargs + a.args]
        if a.vararg:
            names.append('*' + a.vararg.arg)
        names += [x.arg for x in a.kwonlyargs]
        if a.kwarg:
            names.append('**' + a.kwarg.arg)
        pre = 'async ' if isinstance(fn, ast.AsyncFunctionDef) else ''
        return f'{pre}{fn.name}({", ".join(names)})'

    for node in tree.body:
        if isinstance(node, ast.Import):
            imports += [a.name for a in node.names]
        elif isinstance(node, ast.ImportFrom):
            imports.append('.' * node.level + (node.module or '') + ':' + ','.join(a.name for a in node.names)[:60])
        elif isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            d = ast.get_docstring(node)
            out.append('fn ' + sig(node) + (f'  # {ws(d.splitlines()[0], 90)}' if d else ''))
        elif isinstance(node, ast.ClassDef):
            bases = ', '.join(ast.unparse(b) for b in node.bases) if hasattr(ast, 'unparse') else ''
            d = ast.get_docstring(node)
            out.append(f'class {node.name}' + (f'({bases})' if bases else '') + (f'  # {ws(d.splitlines()[0], 90)}' if d else ''))
            for sub in node.body:
                if isinstance(sub, (ast.FunctionDef, ast.AsyncFunctionDef)) and (not sub.name.startswith('_') or sub.name == '__init__'):
                    out.append('  ' + sig(sub))
        elif isinstance(node, ast.If) and 'main' in ast.dump(node.test):
            out.append('entry: if __name__ == "__main__"')
    return imports, out, (ws(doc.splitlines()[0], 200) if doc else '')


# ---------------------------------------------------------------- JS / TS / other
def parse_generic(text, ext):
    imports, out = [], []
    if ext in JS_EXT:
        imports = re.findall(r'''(?:import\s[^'"]*?from\s*|import\s*|require\(\s*)['"]([^'"]+)['"]''', text)
        for m in re.finditer(r'^\s*export\s+(default\s+)?(async\s+)?(class|function\*?|const|let|interface|type|enum)\s+(\w+)([^\n{=]*)', text, re.M):
            out.append(f'export {m.group(3)} {m.group(4)}{ws(m.group(5), 100) if m.group(3).startswith("function") else ""}')
        for m in re.finditer(r'^(?:async\s+)?function\s+(\w+)\s*(\([^)]*\))', text, re.M):
            out.append(f'fn {m.group(1)}{ws(m.group(2), 100)}')
    else:
        imports = re.findall(r'^\s*(?:using|import|use)\s+([\w.:{}*, ]+)', text, re.M)[:30]
        for m in re.finditer(r'^\s*(?:public\s+|internal\s+|pub\s+|export\s+)?(?:abstract\s+|static\s+|sealed\s+|partial\s+)*'
                             r'(class|interface|struct|enum|trait|impl|fn|func|object)\s+(\w+)', text, re.M):
            out.append(f'{m.group(1)} {m.group(2)}')
    return imports, out[:80]


# ---------------------------------------------------------------- build files
def parse_build(name, text):
    out = []
    if name == 'CMakeLists.txt' or name.endswith('.cmake'):
        t = re.sub(r'#[^\n]*', '', text)
        for m in re.finditer(r'\b(project|add_library|add_executable|add_subdirectory|target_link_libraries|find_package|qt_add_executable|qt_add_library)\s*\(([^)]*)\)', t, re.I):
            out.append(f'{m.group(1)}({ws(m.group(2), 160)})')
    elif name.endswith(('.pro', '.pri')):
        for m in re.finditer(r'^\s*(TEMPLATE|TARGET|SUBDIRS|QT|LIBS|CONFIG|DEPENDPATH|include)\s*([+\-*]?=|\()\s*((?:[^\n]*\\\n)*[^\n]*)', text, re.M):
            out.append(f'{m.group(1)} {m.group(2)} {ws(m.group(3).replace(chr(92), " "), 160)}')
    elif name == 'package.json':
        for key in ('name', 'main'):
            m = re.search(rf'"{key}"\s*:\s*"([^"]+)"', text)
            if m:
                out.append(f'{key}: {m.group(1)}')
        m = re.search(r'"dependencies"\s*:\s*\{([^}]*)\}', text)
        if m:
            out.append('deps: ' + ', '.join(re.findall(r'"([^"]+)"\s*:', m.group(1)))[:300])
    elif name.endswith('.vcxproj'):
        for m in re.finditer(r'<(ConfigurationType|RootNamespace|ProjectReference Include)[ =>"]*([^<"]+)', text):
            out.append(f'{m.group(1)}: {m.group(2)}')
    else:
        out = [ws(l, 160) for l in text.splitlines() if l.strip()][:15]
    return out[:40]


# ---------------------------------------------------------------- driver
CONF_FILE = 'codemap.conf'
CONF = None
NESTED_SKIPPED = set()


def load_conf(root='.'):
    conf = {'name': os.path.basename(os.path.abspath(root)), 'links': [], 'exclude': []}
    for line in _read(os.path.join(root, CONF_FILE)).splitlines():
        line = line.split('#', 1)[0].strip()
        if '=' not in line:
            continue
        k, v = (x.strip() for x in line.split('=', 1))
        if k == 'name' and v:
            conf['name'] = v
        elif k in ('links', 'exclude'):
            conf[k] += [x.replace('\\', '/').strip('/') for x in v.replace(',', ' ').split()]
    return conf


def is_project(path):
    return os.path.isfile(os.path.join(path, 'codemap.py')) or os.path.isfile(os.path.join(path, CONF_FILE))


def walk(root):
    excl = set((CONF or {}).get('exclude', []))
    for dp, dns, fns in os.walk(root):
        keep = []
        for d in sorted(dns):
            if skip_dir(d) or d.startswith('.'):
                continue
            full = os.path.join(dp, d)
            rel = os.path.relpath(full, '.').replace('\\', '/')
            if d in excl or rel in excl:
                continue
            if is_project(full):  # a separate codemap project (e.g. a library submodule)
                NESTED_SKIPPED.add(rel)
                continue
            keep.append(d)
        dns[:] = keep
        for fn in sorted(fns):
            yield os.path.join(dp, fn)


def classify(path):
    name = os.path.basename(path)
    if name == 'codemap.py':
        return None
    ext = os.path.splitext(name)[1].lower()
    if name in BUILD_NAMES or ext in BUILD_EXT:
        return 'build'
    if ext in CPP_EXT:
        return 'cpp'
    if ext == '.py':
        return 'py'
    if ext in JS_EXT or ext in OTHER_EXT:
        return 'other'
    return None


def clear(path):
    """Remove an old output file; if deleting is not allowed, blank it instead."""
    try:
        os.remove(path)
    except OSError:
        with open(path, 'w', encoding='utf-8') as f:
            f.write('# stale file from a previous run - ignore\n')


def write_parts(outdir, prefix, blocks, part_chars, title):
    parts, cur = [], []
    size = 0
    for b in blocks:
        if cur and size + len(b) > part_chars:
            parts.append(cur)
            cur, size = [], 0
        cur.append(b)
        size += len(b)
    if cur:
        parts.append(cur)
    files = []
    for k, p in enumerate(parts, 1):
        fn = os.path.join(outdir, f'{prefix}_{k:02d}.md')
        with open(fn, 'w', encoding='utf-8') as f:
            f.write(f'# {title} - part {k}/{len(parts)}\n\n' + ''.join(p))
        files.append(fn)
    return files


def sources_hash(root):
    """Content hash of all indexable files under root (paths relative to root, CRLF-normalized,
    so it is identical on every machine regardless of git autocrlf and file mtimes)."""
    import hashlib
    h = hashlib.sha1()
    for path in walk(root):
        if not classify(path):
            continue
        rel = os.path.relpath(path, root).replace('\\', '/')
        try:
            with open(path, 'rb') as f:
                data = f.read()
        except OSError:
            continue
        h.update(rel.encode('utf-8') + b'\0' + data.replace(b'\r\n', b'\n') + b'\0')
    return h.hexdigest()[:16]


def cmd_index(a):
    root = os.path.abspath(a.root)
    if not os.path.isdir(root):
        print(f'ERROR: folder not found: {a.root}')
        sys.exit(2)
    cwd = os.getcwd()
    inside = os.path.commonpath([root, cwd]) == cwd
    if inside and root != cwd and is_project(root):
        print(f'ERROR: {a.root} is a separate codemap project (has its own codemap.py/{CONF_FILE}). '
              f'Index it from its own folder and add it to "links" in {CONF_FILE}.')
        sys.exit(2)
    base = cwd if inside else root  # paths in the index are relative to the project root (cwd)
    if a.out is None:
        slug = os.path.relpath(root, cwd).replace('\\', '/').strip('./').replace('/', '__') if inside else os.path.basename(root)
        a.out = os.path.join('.codemap', slug or '_root')
        if inside and slug:
            _register_folder(slug.replace('__', '/'))
    os.makedirs(a.out, exist_ok=True)
    for old in os.listdir(a.out):
        if re.match(r'(index_\d+|00_overview)\.md$', old):
            clear(os.path.join(a.out, old))
    dir_stats = defaultdict(lambda: [0, 0])
    ext_stats = defaultdict(lambda: [0, 0])
    blocks, build_blocks, errors = [], [], []
    for path in walk(root):
        kind = classify(path)
        if not kind:
            continue
        rel = os.path.relpath(path, base).replace('\\', '/')
        try:
            text = read(path)
        except OSError as e:
            errors.append(f'{rel}: {e}')
            continue
        lines = text.count('\n') + 1
        d = os.path.dirname(os.path.relpath(path, root).replace('\\', '/')) or '.'
        dir_stats[d][0] += 1
        dir_stats[d][1] += lines
        while d not in ('.', ''):
            d = os.path.dirname(d) or '.'
            dir_stats.setdefault(d, [0, 0])
        ext = os.path.splitext(rel)[1].lower() or os.path.basename(rel)
        ext_stats[ext][0] += 1
        ext_stats[ext][1] += lines
        try:
            if kind == 'build':
                body = parse_build(os.path.basename(rel), text)
                build_blocks.append(f'### {rel}\n' + '\n'.join(body) + '\n\n')
                continue
            desc = first_comment(text, kind == 'py')
            if kind == 'cpp':
                inc, body = parse_cpp(text, os.path.splitext(rel)[1].lower() in HDR_EXT, a.private)
            elif kind == 'py':
                r = parse_py(text)
                inc, body = r[0], r[1]
                desc = (r[2] if len(r) > 2 else '') or desc
            else:
                inc, body = parse_generic(text, os.path.splitext(rel)[1].lower())
        except Exception as e:  # never stop on one bad file
            errors.append(f'{rel}: parser error {type(e).__name__}: {e}')
            inc, body, desc = [], [], ''
        b = f'### {rel} ({lines})\n'
        if desc:
            b += f'> {desc}\n'
        if inc:
            b += 'inc: ' + ' '.join(dict.fromkeys(inc)) + '\n'
        b += '\n'.join(body) + ('\n' if body else '') + '\n'
        blocks.append(b)

    ov = [f'# Code overview: {os.path.relpath(root, base) if inside else os.path.basename(root)}\n',
          f'folder: {os.path.relpath(root, base).replace(os.sep, "/") if inside else root}\n',
          f'codemap: v{CODEMAP_VERSION}\n',
          f'sources: {sources_hash(root)}\n',
          f'files indexed: {sum(v[0] for v in ext_stats.values())}, lines: {sum(v[1] for v in ext_stats.values())}\n',
          '\n## By extension (files / lines)\n']
    for e, (c, l) in sorted(ext_stats.items(), key=lambda x: -x[1][1]):
        ov.append(f'{e}: {c} / {l}\n')
    ov.append('\n## Directories (files / lines)\n')
    for d in sorted(dir_stats):
        depth = 0 if d == '.' else d.count('/') + 1
        c, l = dir_stats[d]
        ov.append('  ' * depth + (d.split('/')[-1] if d != '.' else '.') + '/' + (f'  {c} / {l}' if c else '') + '\n')
    if build_blocks:
        ov.append('\n## Build files\n\n' + ''.join(build_blocks))
    if errors:
        ov.append('\n## Errors\n' + '\n'.join(errors) + '\n')
    ov.append('\n## Legend\n(N)=lines, inc=includes/imports, fn=function, # protected, - private, sig=Qt signal, '
              'slot=Qt slot, prop=Q_PROPERTY, alias=typedef/using, impl X: methods defined in this .cpp, '
              'uses=types referenced in the file but declared elsewhere (look them up in other indexes)\n')
    ov_path = os.path.join(a.out, '00_overview.md')
    with open(ov_path, 'w', encoding='utf-8') as f:
        f.write(''.join(ov))
    parts = write_parts(a.out, 'index', blocks, a.part_chars, 'Code index')
    report([ov_path] + parts, errors)
    if NESTED_SKIPPED:
        print('skipped separate codemap projects: ' + ', '.join(sorted(NESTED_SKIPPED))
              + f' (link them in {CONF_FILE})')


def cmd_dump(a):
    os.makedirs(a.out, exist_ok=True)
    for old in os.listdir(a.out):
        if re.match(r'dump_\d+\.md$', old):
            clear(os.path.join(a.out, old))
    blocks, errors = [], []
    for p in a.files:
        try:
            text = read(p)
        except OSError as e:
            errors.append(f'{p}: {e}')
            continue
        ext = os.path.splitext(p)[1].lstrip('.') or 'text'
        blocks.append(f'## {p.replace(chr(92), "/")}\n```{ext}\n{text.rstrip()}\n```\n\n')
    report(write_parts(a.out, 'dump', blocks, a.part_chars, 'File dump'), errors)


def report(files, errors):
    print('CODEMAP DONE')
    for f in files:
        print(f'{f}\t{os.path.getsize(f)} bytes')
    if errors:
        print(f'ERRORS: {len(errors)} (see list above / in 00_overview.md)')
        for e in errors[:20]:
            print('  ' + e)


MAP_FILE = 'CODEMAP.md'
LOCK_FILE = os.path.join('.codemap', 'codemap.lock')
LOCK_STALE_SEC = 300


def _read(path):
    try:
        with open(path, encoding='utf-8') as f:
            return f.read()
    except OSError:
        return ''


def _write(path, text):
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)


def lock_acquire(wait_sec=120):
    """Cooperative lock without deleting files: the lock file holds 'LOCKED <token> <time>' or 'FREE'."""
    import time, uuid
    os.makedirs('.codemap', exist_ok=True)
    token = uuid.uuid4().hex[:8]
    deadline = time.time() + wait_sec
    while True:
        cur = _read(LOCK_FILE).split()
        busy = len(cur) == 3 and cur[0] == 'LOCKED' and time.time() - float(cur[2]) < LOCK_STALE_SEC
        if not busy:
            _write(LOCK_FILE, f'LOCKED {token} {time.time():.0f}')
            time.sleep(0.3)
            if _read(LOCK_FILE).split()[1:2] == [token]:
                return token
        if time.time() > deadline:
            print(f'ERROR: {MAP_FILE} is locked by another session ({LOCK_FILE}); try again later')
            sys.exit(3)
        time.sleep(1)


def lock_release(token):
    if _read(LOCK_FILE).split()[1:2] == [token]:
        _write(LOCK_FILE, 'FREE')


def _split_sections(text):
    """-> (head_lines, [(title, lines)]) split on '## ' headings."""
    head, secs = [], []
    for line in text.split('\n'):
        if line.startswith('## '):
            secs.append((line[3:].strip(), [line]))
        elif secs:
            secs[-1][1].append(line)
        else:
            head.append(line)
    return head, secs


def _anchor(title):
    a = re.sub(r'[^\w\- ]', '', title.lower(), flags=re.U).strip().replace(' ', '-')
    return a


def cmd_section(a):
    map_file = a.map_file
    folder = a.folder.replace('\\', '/').strip('/')
    if a.show:
        _, secs = _split_sections(_read(map_file))
        for t, lines in secs:
            if t == folder:
                print('\n'.join(lines).rstrip())
                return
        _, secs = _split_sections(_read(map_file))
        print(f'(no section "## {folder}" in {map_file}). Sections: ' + ', '.join(t for t, _ in secs if t != 'Оглавление'))
        return
    body = _read(a.file).strip('\n')
    if not body:
        print(f'ERROR: section file is empty or missing: {a.file}')
        sys.exit(2)
    if not body.startswith(f'## {folder}'):
        body = re.sub(r'\A## [^\n]*\n?', '', body)
        body = f'## {folder}\n\n' + body.lstrip('\n')
    if os.path.dirname(os.path.abspath(map_file)) != os.getcwd():
        print(f'ERROR: {map_file} belongs to another project; write its sections from that project folder')
        sys.exit(2)
    token = lock_acquire()
    try:
        text = _read(map_file)  # re-read under the lock: never write a stale copy
        head, secs = _split_sections(text)
        while head and not head[0].strip():
            head.pop(0)
        while head and not head[-1].strip():
            head.pop()
        if os.path.basename(map_file) == MAP_FILE:
            title = f'# Карта кода {CONF["name"]}'
            if head and head[0].startswith('# '):
                head[0] = title
            else:
                head = [title] + ([''] + head if head else [])
            rest = [l for l in head[1:] if not l.startswith(LINKS_PREFIX)]
            while rest and not rest[0].strip():
                rest.pop(0)
            links = [(load_conf(l)['name'], l) for l in CONF['links'] if os.path.isdir(l)]
            head = [head[0]]
            if links:
                head += ['', LINKS_PREFIX + ', '.join(f'[{n}]({l}/{MAP_FILE})' for n, l in links)
                         + ' — `python3 codemap.py find` ищет и в них']
            if rest:
                head += [''] + rest
        new_lines = body.split('\n')
        replaced = False
        for i, (t, _) in enumerate(secs):
            if t == folder:
                secs[i] = (folder, new_lines)
                replaced = True
        if not replaced:
            secs.append((folder, new_lines))
        had_toc = any(t == 'Оглавление' for t, _ in secs) or os.path.basename(map_file) == MAP_FILE
        secs = [(t, l) for t, l in secs if t != 'Оглавление']
        toc = ['## Оглавление', ''] + [f'- [{t}](#{_anchor(t)})' for t, _ in secs]
        out = ['\n'.join(head)] + (['\n'.join(toc)] if had_toc else []) + ['\n'.join(l).strip('\n') for _, l in secs]
        _write(map_file, '\n\n'.join(out).rstrip('\n') + '\n')
    finally:
        lock_release(token)
    print(f'SECTION DONE: "## {folder}" {"replaced" if replaced else "added"} in {map_file}')


LINKS_PREFIX = 'Связанные карты: '


@__import__('contextlib').contextmanager
def in_project(path):
    """Run commands inside a linked codemap project (its own cwd and codemap.conf)."""
    global CONF
    old, old_conf = os.getcwd(), CONF
    os.chdir(path)
    CONF = load_conf()
    try:
        yield
    finally:
        os.chdir(old)
        CONF = old_conf


def linked_projects():
    """-> [(path, name)] of linked projects that exist on this machine; warns about missing ones."""
    out = []
    for l in CONF['links']:
        if os.path.isdir(l):
            out.append((l, load_conf(l)['name']))
        else:
            print(f'WARNING: linked project not found: {l}')
    return out


FOLDERS_FILE = 'codemap.folders'  # tracked in git: which folders are indexed (.codemap/ itself is not)


def _registered_folders():
    return [l.strip() for l in _read(FOLDERS_FILE).splitlines() if l.strip() and not l.startswith('#')]


def _register_folder(folder):
    folders = _registered_folders()
    if folder and folder not in folders:
        folders.append(folder)
        _write(FOLDERS_FILE, '# folders indexed by codemap.py (commit this file; .codemap/ is generated)\n'
               + '\n'.join(sorted(folders)) + '\n')


def _index_dirs():
    """Indexed folders: from codemap.folders (survives git clone) plus any existing .codemap/<dir>."""
    names = {f.replace('/', '__') for f in _registered_folders()}
    if os.path.isdir('.codemap'):
        names |= {n for n in os.listdir('.codemap') if os.path.isdir(os.path.join('.codemap', n)) and not n.startswith('_')}
    return [(n.replace('__', '/'), os.path.join('.codemap', n)) for n in sorted(names)]


def folder_status(folder, d):
    """-> OK | NOINDEX (fresh clone) | OLD (other script version) | CHANGED (sources differ) | MISSING (folder gone)"""
    if not os.path.isdir(folder):
        return 'MISSING'
    if not os.path.isdir(d):
        return 'NOINDEX'
    ov = _read(os.path.join(d, '00_overview.md'))
    if f'codemap: v{CODEMAP_VERSION}' not in ov.split('\n'):
        return 'OLD'
    m = re.search(r'^sources: (\w+)$', ov, re.M)
    if not m or m.group(1) != sources_hash(folder):
        return 'CHANGED'
    return 'OK'


def cmd_status(a, label=''):
    sections = {t for t, _ in _split_sections(_read(MAP_FILE))[1]}
    for folder, d in _index_dirs():
        st = folder_status(folder, d)
        print(f'{label}{st} {folder} section={1 if folder in sections else 0}')
    if not _index_dirs():
        print(f'{label}no indexes in .codemap/')
    if not label:
        for path, name in linked_projects():
            with in_project(path):
                cmd_status(a, label=f'[{name}] ')


def cmd_reindex(a, label=''):
    """Rebuild indexes of already indexed folders (no AI, free). Default: only OLD/CHANGED."""
    done, skipped, missing = [], [], []
    for folder, d in _index_dirs():
        st = folder_status(folder, d)
        if st == 'MISSING':
            missing.append(folder)
            continue
        _register_folder(folder)
        if st == 'OK' and not a.all:
            skipped.append(folder)
            continue
        ns = argparse.Namespace(root=folder, out=d, private=a.private, part_chars=a.part_chars)
        import io, contextlib
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            cmd_index(ns)
        done.append(f'{folder} ({st})')
    if not label:
        print('REINDEX DONE')
    print(f'{label}reindexed: ' + (', '.join(done) or '-'))
    print(f'{label}up to date: ' + (', '.join(skipped) or '-'))
    if missing:
        print(f'{label}missing folders (index kept): ' + ', '.join(missing))
    if not label and not a.no_links:
        for path, name in linked_projects():
            with in_project(path):
                cmd_reindex(a, label=f'[{name}] ')


def _find_in(name, prefix):
    """Scan this project's indexes and CODEMAP.md; paths get `prefix` (path of a linked project)."""
    import glob
    pat = re.compile(r'(?<![\w])' + re.escape(name) + r'(?![\w])')
    n = re.escape(name)
    decl, impl, uses, other, hits = [], [], [], [], []
    for idx in sorted(glob.glob(os.path.join('.codemap', '*', 'index_*.md'))):
        cur = None
        for line in open(idx, encoding='utf-8', errors='replace'):
            line = line.rstrip('\n')
            if line.startswith('### '):
                cur = prefix + line[4:]
                continue
            if not cur or not pat.search(line):
                continue
            s = line.strip()
            item = f'{cur}: {s[:200]}'
            if re.match(rf'(class|struct|union|enum|alias|namespace)\s+(\w+::)*{n}\b', s) or \
                    re.match(rf'(fn|sig|slot|prop|# |- )?\s*[^(]*\b{n}\s*\(', s) and not s.startswith(('uses:', 'inc:', 'impl ')) and \
                    re.match(r'(fn|sig|slot|prop|# |- )', s):
                decl.append(item)
            elif s.startswith('impl '):
                impl.append(item)
            elif s.startswith(('uses:', 'inc:')):
                uses.append(cur.split(' (')[0])
            else:
                other.append(item)
    if os.path.exists(MAP_FILE):
        section = ''
        for k, line in enumerate(open(MAP_FILE, encoding='utf-8'), 1):
            if line.startswith('## '):
                section = line[3:].strip()
            if pat.search(line) and not line.startswith(LINKS_PREFIX):
                hits.append(f'{prefix}{MAP_FILE}:{k} [## {section}] {line.strip()[:160]}')
    return decl, impl, other, sorted(set(uses)), hits


def cmd_find(a):
    """Look a name up in all indexes and maps (this project + linked ones)."""
    projects = [('', CONF['name'])] + [(p + '/', nm) for p, nm in linked_projects()]
    found = False
    for prefix, pname in projects:
        if prefix:
            with in_project(prefix.rstrip('/')):
                res = _find_in(a.name, prefix)
        else:
            res = _find_in(a.name, '')
        if not any(res):
            continue
        found = True
        decl, impl, other, uses, hits = res
        if len(projects) > 1:
            print(f'=== {pname}' + (f' ({prefix.rstrip("/")})' if prefix else ''))
        def show(title, items, limit):
            if items:
                print(f'== {title} ({len(items)})')
                for it in items[:limit]:
                    print('  ' + it)
                if len(items) > limit:
                    print(f'  ... +{len(items) - limit}')
        show('declared', decl, 20)
        show('implemented in', impl, 10)
        show('members/signatures mentioning it', other, 15)
        show('used by files', uses, 30)
        show(f'{prefix}{MAP_FILE}', hits, 8)
    if not found:
        where = ' and linked projects' if len(projects) > 1 else ''
        print(f'NOT FOUND in .codemap indexes{where}. Try: grep -rn -w "{a.name}" --include=*.h --include=*.hpp --include=*.cpp .')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--version', action='version', version=f'codemap: v{CODEMAP_VERSION}')
    sub = ap.add_subparsers(dest='cmd', required=True)
    pi = sub.add_parser('index')
    pi.add_argument('root', nargs='?', default='.')
    pi.add_argument('--private', action='store_true', help='include private methods')
    pd = sub.add_parser('dump')
    pd.add_argument('files', nargs='+')
    pi.add_argument('--out', default=None, help='default: .codemap/<folder path with __>')
    pd.add_argument('--out', default='.codemap/_dump')
    for p in (pi, pd):
        p.add_argument('--part-chars', type=int, default=150000)
    sub.add_parser('status', help='OK/OLD/CHANGED/MISSING for every indexed folder (content hash, not mtime)')
    pr = sub.add_parser('reindex', help='rebuild indexes of indexed folders that are OLD/CHANGED (free, no AI)')
    pr.add_argument('--all', action='store_true', help='rebuild every index')
    pr.add_argument('--private', action='store_true')
    pr.add_argument('--part-chars', type=int, default=150000)
    pr.add_argument('--no-links', action='store_true', help='do not reindex linked projects')
    pf = sub.add_parser('find', help='where a class/function/alias is declared, implemented and used')
    pf.add_argument('name')
    ps = sub.add_parser('section', help='write/replace one "## FOLDER" section of CODEMAP.md under a lock')
    ps.add_argument('folder')
    ps.add_argument('file', nargs='?', help='markdown file with the section text')
    ps.add_argument('--show', action='store_true', help='print the current section and exit')
    ps.add_argument('--file', dest='map_file', default=MAP_FILE, help='markdown file to work with (default CODEMAP.md), e.g. CONVENTIONS.md')
    a = ap.parse_args()
    global CONF
    CONF = load_conf()
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    if a.cmd == 'section' and not a.show and not a.file:
        ap.error('section: give FILE or --show')
    {'index': cmd_index, 'dump': cmd_dump, 'section': cmd_section, 'find': cmd_find,
     'status': cmd_status, 'reindex': cmd_reindex}[a.cmd](a)


if __name__ == '__main__':
    main()
