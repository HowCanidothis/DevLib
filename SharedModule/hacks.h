#ifndef HACKS_H
#define HACKS_H

#ifndef __MINGW32__

#define ADD_HACK_PRIVATE_METHOD(Namespace, Type, VictimClassName, VictimMemberName) \
typedef Type (VictimClassName::* Namespace##VictimClassName); \
template<typename Namespace##VictimClassName, Namespace##VictimClassName M> \
struct Namespace##Class \
{ \
    template<class T> \
    friend Type& Namespace(const T& test) \
    { \
        return const_cast<Type&>(test.*M); \
    } \
}; \
template struct Namespace##Class<Namespace##VictimClassName, &VictimClassName::VictimMemberName>

#define ADD_HACK_PRIVATE_METHOD_ARRAY(Namespace, Type, VictimClassName, VictimMemberName, ArrayDim) \
typedef Type (VictimClassName::* Namespace##VictimClassName) ArrayDim; \
template<typename Namespace##VictimClassName, Namespace##VictimClassName M> \
struct Namespace##Class \
{ \
    template<class T> \
    friend Type** Namespace(const T& test) \
    { \
        auto& pp = test.*M; \
        return reinterpret_cast<Type**>(pp); \
    } \
}; \
template struct Namespace##Class<Namespace##VictimClassName, &VictimClassName::VictimMemberName>

#endif

/*
    Usage:
class Test
{
    char* array;
    float mer[4][4];
    int flagBits;
public:
    Test(int flagBits)
        : flagBits(flagBits)
    {}

    void SetArray(char* value)
    {
        array = value;
    }

    void print()
    {
        qDebug() << flagBits << mer[0][0] << mer[3][0] << mer[3][3] << (void*)array;
    }
};
ADD_HACK_METHOD_ARRAY(HackArray, float, Test, mer, [4][4]);
ADD_HACK_METHOD(HackInt, int, Test, flagBits);
ADD_HACK_METHOD(HackChar, char*, Test, array);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Test test(123);

    HackInt(test) = 12;
    auto*& HackChar(test);
    auto** HackArray(test);
    return a.exec();
}
*/
#endif // HACKS_H
