#ifndef __SPOT_H__
#define __SPOT_H__

class Spot {

private:
	QPoint distance;
	QPoint position;
	QImage image;
	QRect target;
	QRect source;

public:
	Spot();

	void setDistance(QPoint distance);
	QPoint getDistance();

	void setPosition(QPoint position);
	QPoint getPosition();

	QRect getTarget();
	QRect getSource();
	QImage getImage();
};

#endif