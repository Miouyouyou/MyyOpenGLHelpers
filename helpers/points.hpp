#pragma once

#include <iostream>

namespace myy {
	template <typename T>
	struct Point2D {
		typedef T ComponentType;
		T x, y;
		Point2D() :
			x((T)0), y((T)0)
		{ }
		Point2D(T const cx, T const cy) :
			x(cx), y(cy)
		{ }
		Point2D(T const * __restrict const coords) :
			x(coords[0]),
			y(coords[1])
		{ }
		Point2D(Point2D const & op) :
			x(op.x),
			y(op.y)
		{ }
		Point2D & operator=(Point2D & op)
		{ 
			x = op.x; y = op.y;
			return *this;
		}
		Point2D & operator=(Point2D const & op)
		{
			x = op.x; y = op.y;
			return *this;
		}
		Point2D & operator=(Point2D && op)
		{
			x = op.x; y = op.y;
			return *this;
		}
		Point2D & operator=(Point2D const && op)
		{
			x = op.x; y = op.y;
			return *this;
		}

		Point2D operator+(Point2D const op) const
		{ return Point2D(x+op.x, y+op.y); }

		Point2D operator+(T n) const
		{ return Point2D(x+n, y+n); }

		Point2D operator+(T const * __restrict const coords)
		{ return Point2D(x+coords[0], y+coords[1]); }

		Point2D operator-(Point2D const op) const
		{ return Point2D(x-op.x, y-op.y); }

		T h() const { return x; }
		T h(T new_h) { x = new_h; return x; }
		T v() const { return y; }
		T v(T new_v) { y = new_v; return y; }

		Point2D mix_hv(Point2D const other_point) const {
			Point2D new_point(h(), other_point.v());
			return new_point;
		}

		Point2D mix_hv(Point2D const * __restrict const other_point) const {
			Point2D new_point(h(), other_point->v());
			return new_point;
		}

		void log(std::ostream & stream) const {
			stream 
				<< "Point2D : {.x = " << x << ", y = " << y << "};\n";
		}
	};

	template <typename T>
	struct Point3D {
		typedef T ComponentType;
		T x, y, z;
		Point3D() :
			x((T)0), y((T)0), z((T)0)
		{ }
		Point3D(T const ox, T const oy, T const oz) :
			x(ox), y(oy), z(oz)
		{ }
		Point3D(T const * __restrict const coords) :
			x(coords[0]),
			y(coords[1]),
			z(coords[2])
		{ }
		Point3D(Point3D const & op) :
			x(op.x),
			y(op.y),
			z(op.z)
		{ }
		Point3D(position_S_3D const pos) :
			x(pos.x),
			y(pos.y),
			z(pos.z)
		{ }
		Point3D & operator=(Point3D & op)
		{ 
			x = op.x; y = op.y; z = op.z;
			return *this;
		}
		Point3D & operator=(Point3D const & op)
		{
			x = op.x; y = op.y; z = op.z;
			return *this;
		}
		Point3D & operator=(Point3D && op)
		{
			x = op.x; y = op.y; z = op.z;
			return *this;
		}
		Point3D & operator=(Point3D const && op)
		{
			x = op.x; y = op.y; z = op.z;
			return *this;
		}

		T h() const { return x; }
		T h(T new_h) { x = new_h; return x; }
		T v() const { return y; }
		T v(T new_v) { y = new_v; return y; }
		T p() const { return z; }
		T p(T new_p) { z = new_p; return p; }

		Point3D mix_hv(Point3D const other_point) const {
			Point3D new_point(h(), other_point.v(), p());
			return new_point;
		}

		Point3D mix_hv(Point3D const * __restrict const other_point) const {
			Point3D new_point(h(), other_point->v(), p());
			return new_point;
		}

		void log(std::ostream & stream) const {
			stream 
				<< "Point3D : {x = " << x << ", y = " << y << ", z = " << z << "};\n";
		}

	};

	template <typename PointType>
	struct Triangle {
		PointType a, b, c;
		Triangle() :
			a(PointType()),
			b(PointType()),
			c(PointType())
		{ std::cerr << "Triangle : Called default constructor\n"; }

		Triangle(
			PointType const pa,
			PointType const pb,
			PointType const pc) :
			a(pa), b(pb), c(pc)
		{ std::cerr << "Triangle : Called three points constructor\n"; }
		Triangle(PointType const * __restrict const coords) :
			a(coords[0]),
			b(coords[1]),
			c(coords[2])
		{ std::cerr << "Triangle : Called points array constructor\n"; }
		Triangle(Triangle const & ot) :
			a(ot.a),
			b(ot.b),
			c(ot.c)
		{ std::cerr << "Triangle : Constructing from another one\n"; }
		Triangle & operator=(Triangle & ot)
		{ 
			std::cerr << "Triangle : Affectation constructor\n";
			a = ot.a; b = ot.b; c = ot.c;
			return *this;
		}
		Triangle & operator=(Triangle && ot)
		{ 
			std::cerr << "Triangle : Move constructor";
			a = ot.a; b = ot.b; c = ot.c;
			return *this;
		}

		Triangle operator+(Triangle const ot) const
		{
			return Triangle(a+ot.a, b+ot.b, c+ot.c);
		}

		Triangle operator+(PointType const * __restrict const coords)
		{
			return Triangle(a+coords[0], b+coords[1], c+coords[2]);
		}

		Triangle operator-(Triangle const ot) const
		{ 
			return Triangle(a-ot.x, b-ot.b, c-ot.c);
		}

		void log(std::ostream & stream) const {
			stream << "Triangle : {\n";
			a.log(stream);
			b.log(stream);
			c.log(stream);
			stream << "}\n";
		}

		PointType * points() {
			return (PointType *) this;
		}

		PointType const * points_const() const {
			return (PointType const *) this;
		}

		constexpr inline unsigned int n_points() { return 3; }
	};
}

