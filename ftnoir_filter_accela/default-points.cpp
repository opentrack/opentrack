#include <QList>
#include <QPointF>

static QList<QPointF> EmptyList() {
	return QList<QPointF>();
}

extern const QList<QPointF> defScaleRotation, defScaleTranslation;

const QList<QPointF> defScaleRotation =
	EmptyList()
    << QPointF(0, 0)
    << QPointF(0.308900523560209, 0.0666666666666667)
    << QPointF(0.565445026178011, 0.226666666666667)
    << QPointF(0.769633507853403, 0.506666666666667)
    << QPointF(0.994764397905759, 1)
    << QPointF(1.23560209424084, 1.61333333333333)
    << QPointF(1.47643979057592, 2.37333333333333)
    << QPointF(1.66492146596859, 3.12)
    << QPointF(1.80628272251309, 3.92)
    << QPointF(1.91623036649215, 4.70666666666667)
    << QPointF(2.00523560209424, 5.44)
    << QPointF(2.07329842931937, 6)
;

const QList<QPointF> defScaleTranslation =
	EmptyList()
    << QPointF(0, 0)
    << QPointF(0.282722513089005, 0.08)
    << QPointF(0.492146596858639, 0.306666666666667)
    << QPointF(0.764397905759162, 0.84)
    << QPointF(1.00523560209424, 1.62666666666667)
    << QPointF(1.17277486910995, 2.78666666666667)
    << QPointF(1.25130890052356, 3.6)
    << QPointF(1.31937172774869, 4.29333333333333)
    << QPointF(1.38219895287958, 4.90666666666667)
    << QPointF(1.43455497382199, 5.65333333333333)
;
