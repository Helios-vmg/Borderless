public:
	UintPair(unsigned x = 0, unsigned y = 0) : x(x), y(y) {}
	UintPair(const QSize &size) : UintPair(size.width(), size.height()) {}
	QSize to_QSize() const{
		return QSize(this->x, this->y);
	}
