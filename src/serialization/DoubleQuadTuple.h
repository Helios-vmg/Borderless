public:
	DoubleQuadTuple(double x0 = 1, double x1 = 0, double x2 = 0, double x3 = 1) : x0(x0), x1(x1), x2(x2), x3(x3) {}
	DoubleQuadTuple(const QMatrix &mat): DoubleQuadTuple(mat.m11(), mat.m21(), mat.m12(), mat.m22()){}
	QMatrix to_QMatrix() const{
		return QMatrix(this->x0, this->x2, this->x1, this->x3, 0, 0);
	}
