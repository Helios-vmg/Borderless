public:
	IntPair(int x = 0, int y = 0): x(x), y(y){}
	IntPair(const QPoint &point): IntPair(point.x(), point.y()){}
	QPoint to_QPoint() const{
		return QPoint(this->x, this->y);
	}