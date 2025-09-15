#ifndef MATH_H
#define MATH_H


namespace SLMath
{

template<class Eps>
struct HashDouble
{
    HashDouble(double t=0.0)
        : Value(t)
    {}

    double Value;

    bool operator==(const HashDouble& another) const
    {
        return fuzzyCompare(another.Value, Value, Eps::Value);
    }
    operator double() const { return Value; }
};

template<class T, class Eps>
class Geometry
{
public:
    using FGeometryHandler = std::function<T (double t)>;

    Geometry()
    {}

    Geometry(const FGeometryHandler& curve)
        : m_curve(curve)
    {}

    T At(double t) const
    {
        auto foundIt = m_cache.find(t);
        if(foundIt != m_cache.end()) {
            return foundIt.value();
        }
        auto v = m_curve(t);
        m_cache.insert(t, v);
        return v;
    }

    std::pair<HashDouble<Eps>, double> ClosestPointDistance(const T& p, const std::function<double (const T&,const T&)>& handler) const
    {
        auto left = 0.0, right = 1.0;
        auto eps = std::numeric_limits<double>().max();
        auto dist = std::numeric_limits<double>().max();
        do {
            auto d = chooseDirection(p, handler, left, right);
            dist = d.second;
            eps = d.first;
        } while(!fuzzyIsNull(eps, Eps::Value));
        return std::make_pair(left, dist);
    }

private:
    std::pair<double,double> chooseDirection(const T& p, const std::function<double (const T&,const T&)>& handler, double& tLeft, double& tRight) const
    {
        auto quart = (tRight - tLeft) / 4.0;
        auto mid = (tLeft + tRight) / 2.0;
        auto l = mid - quart;
        auto r = mid + quart;

        auto left = At(l);
        auto right = At(r);
        auto result = handler(left,right);
        auto dl = handler(left, p);
        auto dr = handler(right, p);
        double min;
        if(dl < dr) {
            tRight = mid;
            min = dl;
        } else {
            tLeft = mid;
            min = dr;
        }
        return std::make_pair(result,min);
    }

private:
    FGeometryHandler m_curve;
    mutable QHash<HashDouble<Eps>, T> m_cache;
};

struct Epsilon01
{
    const static double Value;
};

const double Epsilon01::Value = 0.01;

template<class T, typename Eps>
inline std::pair<T, double> ClosestPoint(const T& p, const std::function<double (const T&, const T&)>& distHandler, const QVector<Geometry<T, Eps>>& geometries, qint32* index = nullptr)
{
    auto result = std::make_pair(T(), std::numeric_limits<double>().max());
    auto i = 0;
    for(const Geometry<T, Eps>& m : geometries) {
        auto tmp = m.ClosestPointDistance(p, distHandler);
        if(result.second > tmp.second) {
            result.first = m.At(tmp.first);
            result.second = tmp.second;
            if(index != nullptr) {
                *index = i;
            }
        }
        ++i;
    }
    return result;
}

inline std::pair<QPointF, double> ClosestPoint(const QPointF& p, const QVector<Geometry<QPointF, Epsilon01>>& geometries, qint32* index)
{
    return ClosestPoint<QPointF, Epsilon01>(p, [](const QPointF& p1, const QPointF& p2) {
        return (p2 - p1).manhattanLength();
    }, geometries, index);
}

}

#endif // MATH_H
