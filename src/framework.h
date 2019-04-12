/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	Here we define all the mathematical classes like Vector3, Matrix44 and some extra useful geometrical functions
*/

#ifndef FRAMEWORK //macros to ensure the code is included once
#define FRAMEWORK

#include <vector>
#include <cmath>
#include <cassert>

#define DEG2RAD 0.0174532925
#define RAD2DEG 57.295779513
#ifndef PI
	#define PI 3.14159265359
#endif

//more standard type definition
typedef char int8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short int16;
typedef int int32;
typedef unsigned int uint32;

inline float random(float range = 1.0f, float offset = 0.0f) { return ((rand() % 10000) / (10000.0f)) * range + offset; }
inline float clamp(float a, float min, float max) { return a < min ? min : (a > max ? max : a); }
inline float lerp(float a, float b, float f) { return a * (1.0f - f) + b * f; }
inline float smoothstep(float edge0, float edge1, float x) { x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); return x * x * (3 - 2 * x); }

//template for vector 2 classes
template <typename T>
class tVector2 {
public:
	union
	{
		struct { T x, y; };
		T v[2];
	};
	tVector2() { x = y = 0; }
	template<class T2> tVector2(const T2& c) { x = (T)c.x; y = (T)c.y; }
	tVector2(T x, T y) { this->x = (T)x; this->y = (T)y; }
	void set(T x, T y) { this->x = (T)x; this->y = (T)y; }
	double length() { return sqrt(x*x + y*y); }
	double length() const { return sqrt(x*x + y*y); }
	template<class T2> double distance(const T2& v) const { return (*this - v).length(); }
	int size() const { return 2; }
	void operator *= (float v) { x*=v; y*=v; }
	void operator /= (float v) { x /= v; y /= v; }
	template<class T2> void operator += (const T2& v) { x += v.x; y += v.y; }
	template<class T2> void operator -= (const T2& v) { x -= v.x; y -= v.y; }
	template<class T2> void operator = (const T2& v) { x = v.x; y = v.y; }
};

template<typename T> bool operator == (const tVector2<T>& a, const tVector2<T>& b) { return a.x == b.x && a.y == b.y; }
template<typename T> inline tVector2<T> operator + (const tVector2<T>& a, const tVector2<T>& b) { return tVector2<T>(a.x + b.x, a.y + b.y); }
template<typename T> inline tVector2<T> operator - (const tVector2<T>& a, const tVector2<T>& b) { return tVector2<T>(a.x - b.x, a.y - b.y); }
template<typename T> inline tVector2<T> operator * (const tVector2<T>& a, const float& f) { return tVector2<T>(a.x * f, a.y * f); }

/*
Vector2 operator * (const Vector2& a, float v);
Vector2 operator / (const Vector2& a, float v);
Vector2 operator + (const Vector2& a, const Vector2& b);
Vector2 operator - (const Vector2& a, const Vector2& b);
Vector2 rotate(const Vector2& a, float degs);
Vector2 normalize(const Vector2& v);
float dot(const Vector2& a, const Vector2& b);
float perpdot(const Vector2& a, const Vector2& b);
Vector2 reflect(const Vector2&a, const Vector2& n);
*/

typedef tVector2<uint8> Vector2ub;
typedef tVector2<int16> Vector2s;
typedef tVector2<int32> Vector2i;

//specific vector2 class
class Vector2
{
public:
	union
	{
		struct { float x,y; };
		float v[2];
	};

	typedef float value_type;

	Vector2() { x = y = 0.0f; }
	Vector2(float x, float y) { this->x = x; this->y = y; }
	template <typename T> Vector2(const tVector2<T>& v) { x = v.x; y = v.y; }

	double length() { return sqrt(x*x + y*y); }
	double length() const { return sqrt(x*x + y*y); }
	int size() const { return 2; }

	float dot( const Vector2& v ) const;
	float perpdot( const Vector2& v ) const;

	void set(float x, float y) { this->x = x; this->y = y; }

	Vector2& normalize() { *this /= (float)length(); return *this; }

	float distance(const Vector2& v);
	Vector2& random(float range);
	void parseFromText(const char* text);
	std::string toString();

	template <typename T> void operator = (const tVector2<T>& v) { x = v.x; y = v.y; }
	void operator *= (float v) { x*=v; y*=v; }
	void operator /= (float v) { x /= v; y /= v; }
	void operator += (const Vector2& v) { x += v.x; y += v.y; }
	void operator -= (const Vector2& v) { x -= v.x; y -= v.y; }
};

bool operator == (const Vector2& a, const Vector2& b);
Vector2 operator * (const Vector2& a, float v);
Vector2 operator / (const Vector2& a, float v);
Vector2 operator + (const Vector2& a, const Vector2& b);
Vector2 operator - (const Vector2& a, const Vector2& b);
Vector2 rotate(const Vector2& a, float degs);
Vector2 normalize(const Vector2& v);
float dot(const Vector2& a, const Vector2& b);
float perpdot(const Vector2& a, const Vector2& b);
Vector2 reflect(const Vector2&a, const Vector2& n);


template<typename T>
class tVector3
{
public:
	union
	{
		struct { T x;
				 T y;
				 T z; };
		T v[3];
		struct {
			T r;
			T g;
			T b;
		};
	};
	tVector3() { x = y = z = 0; }
	tVector3(const tVector3<T>& c) { x = c.x; y = c.y; z = c.z; }
	tVector3(T x, T y, T z) { this->x = x; this->y = y; this->z = z; }
	void set(T x, T y, T z) { this->x = x; this->y = y; this->z = z; }
};

typedef tVector3<uint32> Vector3i;
typedef tVector3<int32> Vector3u;

//*********************************

class Vector3
{
public:
	union
	{
		struct { float x,y,z; };
		float v[3];
		struct { Vector2 xy; };
	};

	Vector3() : xy() { x = y = z = 0.0f; }
	Vector3(float x, float y, float z) : xy() { this->x = x; this->y = y; this->z = z;	}

	double length();
	double length() const;

	void set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }

	void setMin(const Vector3 & v);
	void setMax(const Vector3 & v);

	Vector3& normalize();
	void random(float range);
	void random(Vector3 range);

	float distance(const Vector3& v) const;

	Vector3 cross( const Vector3& v ) const;
	float dot( const Vector3& v ) const;

	void parseFromText(const char* text, const char separator);
	std::string toString();

	float& operator [] (int n) { return v[n]; }
	void operator = (float* v) { assert(v); x = v[0]; y = v[1]; z = v[2]; }
	void operator *= (float v) { x *= v; y *= v; z *= v; }
	void operator += (const Vector3& v) { x += v.x; y += v.y; z += v.z; }
	void operator -= (const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; }

	Vector2 XZ() { return Vector2(x, z); }
	Vector2 XY() { return Vector2(x, y); }
};

Vector3 normalize(const Vector3& n);
float dot( const Vector3& a, const Vector3& b);
Vector3 cross(const Vector3&a, const Vector3& b);
Vector3 rotateY( const Vector3& a, float degs);
Vector3 rotate( const Vector3& a, Vector3 axis, float degs);

inline Vector3 operator + (const Vector3& a, const Vector3& b) { return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3 operator - (const Vector3& a, const Vector3& b) { return Vector3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3 operator * (const Vector3& a, const Vector3& b) { return Vector3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline Vector3 operator * (const Vector3& a, float v) { return Vector3(a.x * v, a.y * v, a.z * v); }
inline Vector3 operator * (float v, const Vector3& a) { return Vector3(a.x * v, a.y * v, a.z * v); }
inline Vector3 operator / (const Vector3& a, float v) { return Vector3(a.x / v, a.y / v, a.z / v); }

inline Vector2 lerp(Vector2 a, Vector2 b, float f) { return a * (1.0f - f) + b * f; }
inline Vector3 lerp(Vector3 a, Vector3 b, float f) { return a * (1.0f - f) + b * f; }


class Vector4
{
public:
	union
	{
		struct { float x,y,z,w; };
		float v[4];
		struct { Vector3 xyz; float _w;  };
	};

	Vector4() { x = y = z = w = 0.0; }
	Vector4(float x, float y, float z, float w = 1.0f) { this->x = x; this->y = y; this->z = z; this->w = w; }
	Vector4(const Vector3& v, float w) { x = v.x; y = v.y; z = v.z; this->w = w; }
	Vector4(const float* v) { x = v[0]; x = v[1]; x = v[2]; x = v[3]; }
	void set(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
	bool isZero() const { return x == 0 && y == 0 && z == 0 && w == 0; }
};

inline Vector4 operator * (const Vector4& a, float v) { return Vector4(a.x * v, a.y * v, a.z * v, a.w * v); }


//Color class to store colors in unsigned byte with alpha
class Color
{
public:
	union
	{
		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		};
		unsigned char v[4];
	};
	Color() { r = g = b = 0; a = 255; }
	Color(unsigned int hex, unsigned char alpha = 255) { b = 0xFF & hex; g = (0xFF00 & hex) >> 8; r = (0xFF0000 & hex) >> 16; a = alpha; } //use hex notation Color(0xFF0000)
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) { this->r = r; this->g = g; this->b = b; this->a = a; }
	void operator = (const Vector3& v);

	void set(float r, float g, float b, float a = 255.0) { this->r = (unsigned char)clamp(r, 0.0, 255.0); this->g = (unsigned char)clamp(g, 0.0, 255.0); this->b = (unsigned char)clamp(b, 0.0, 255.0); this->a = (unsigned char)clamp(a, 0.0, 255.0); }
	void random() { r = rand() % 255; g = rand() % 255; b = rand() % 255; }
	static Color RANDOM() { return Color((unsigned char)(rand() % 255), (unsigned char)(rand() % 255), (unsigned char)(rand() % 255)); }

	Color operator * (float v) { return Color((unsigned char)(r*v), (unsigned char)(g*v), (unsigned char)(b*v)); }
	void operator *= (float v) { r = (unsigned char)(r * v); g = (unsigned char)(g * v); b = (unsigned char)(b * v); }
	Color operator / (float v) { return Color((unsigned char)(r / v), (unsigned char)(g / v), (unsigned char)(b / v)); }
	void operator /= (float v) { r = (unsigned char)(r / v); g = (unsigned char)(g / v); b = (unsigned char)(b / v); }
	Color operator + (const Color& v) { return Color((unsigned char)clamp(r + v.r, 0, 255), (unsigned char)clamp(g + v.g, 0, 255), (unsigned char)clamp(b + v.b, 0, 255)); }
	void operator += (const Color& v) { r += v.r; g += v.g; b += v.b; }
	Color operator - (const Color& v) { return Color((float)(r - v.r), (float)(g - v.g), (float)(b - v.b)); }
	void operator -= (const Color& v) { r -= v.r; g -= v.g; b -= v.b; }

	//some colors to help
	static const Color WHITE;
	static const Color BLACK;
	static const Color GRAY;
	static const Color RED;
	static const Color GREEN;
	static const Color BLUE;
	static const Color CYAN;
	static const Color YELLOW;
	static const Color PURPLE;
};

inline Color lerp(const Color& a, const Color& b, float f) { return Color((unsigned char)(a.r*(1.0 - f) + b.r*f), (unsigned char)(a.g*(1.0 - f) + b.g*f), (unsigned char)(a.b*(1.0 - f) + b.b*f)); }
inline Color blendColors(const Color& a, const Color& b) { float f = 1.0 - a.a / 255.0;  return Color((unsigned char)(a.r*(1.0 - f) + b.r*f), (unsigned char)(a.g*(1.0 - f) + b.g*f), (unsigned char)(a.b*(1.0 - f) + b.b*f)); }
inline Color operator * (const Color& c, float v) { return Color((unsigned char)(c.r*v), (unsigned char)(c.g*v), (unsigned char)(c.b*v)); }
inline Color operator * (float v, const Color& c) { return Color((unsigned char)(c.r*v), (unsigned char)(c.g*v), (unsigned char)(c.b*v)); }
inline Color operator * (const Color& a, const Color& b) { return Color((a.r / 255.0) * (b.r / 255.0) * 255, (a.g / 255.0) * (b.g / 255.0) * 255, (a.b / 255.0) * (b.b / 255.0) * 255); } //ignores alpha

//****************************
//Matrix44 class
class Matrix44
{
	public:
		static const Matrix44 IDENTITY;

		//This matrix works in 
		union { //allows to access the same var using different ways
			struct
			{
				float        _11, _12, _13, _14;
				float        _21, _22, _23, _24;
				float        _31, _32, _33, _34;
				float        _41, _42, _43, _44;
			};
			float M[4][4]; //[row][column]
			float m[16];
		};

		Matrix44();
		Matrix44(const float* v);

		void set(); //multiply with opengl matrix
		void load(); //load in opengl matrix
		void clear();
		void setIdentity();
		void transpose();

		//get base vectors
		Vector3 rightVector() { return Vector3(m[0],m[1],m[2]); }
		Vector3 topVector() { return Vector3(m[4],m[5],m[6]); }
		Vector3 frontVector() { return Vector3(m[8],m[9],m[10]); }

		bool inverse();
		void setUpAndOrthonormalize(Vector3 up);
		void setFrontAndOrthonormalize(Vector3 front);

		Matrix44 getRotationOnly(); //used when having scale

		//rotate only
		Vector3 rotateVector( const Vector3& v);

		//transform using local coordinates
		void translate(float x, float y, float z);
		void rotate( float angle_in_rad, const Vector3& axis  );
		void scale(float x, float y, float z);

		//transform using global coordinates
		void translateGlobal(float x, float y, float z);
		void rotateGlobal( float angle_in_rad, const Vector3& axis  );

		//create a transformation matrix from scratch
		void setTranslation(float x, float y, float z);
		void setRotation( float angle_in_rad, const Vector3& axis );
		void setScale(float x, float y, float z);

		Vector3 getTranslation();

		bool getXYZ(float* euler) const;

		void lookAt(Vector3& eye, Vector3& center, Vector3& up);
		void perspective(float fov, float aspect, float near_plane, float far_plane);
		void ortho(float left, float right, float bottom, float top, float near_plane, float far_plane);

		Vector3 project(const Vector3& v);

		//old fixed pipeline (do not used if possible)
		void multGL();
		void loadGL();

		Matrix44 operator * (const Matrix44& matrix) const;
};

//Operators, they are our friends
//Matrix44 operator * ( const Matrix44& a, const Matrix44& b );
Vector3 operator * (const Matrix44& matrix, const Vector3& v);
Vector4 operator * (const Matrix44& matrix, const Vector4& v); 


class Quaternion
{
public:

	union
	{
		struct { float x; float y; float z; float w; };
		float q[4];
	};

public:
	Quaternion();
	Quaternion(const float* q);
	Quaternion(const Quaternion& q);
	Quaternion(const float X, const float Y, const float Z, const float W);
	Quaternion(const Vector3& axis, float angle);

	void identity();
	Quaternion invert() const;
	Quaternion conjugate() const;

	void set(const float X, const float Y, const float Z, const float W);
	void slerp(const Quaternion& b, float t);
	void slerp(const Quaternion& q2, float t, Quaternion &q3) const;

	void lerp(const Quaternion& b, float t);
	void lerp(const Quaternion& q2, float t, Quaternion &q3) const;

public:
	void setAxisAngle(const Vector3& axis, const float angle);
	void setAxisAngle(float x, float y, float z, float angle);
	void getAxisAngle(Vector3 &v, float &angle) const;

	Vector3 rotate(const Vector3& v) const;

	void operator*=(const Vector3& v);
	void operator *= (const Quaternion &q);
	void operator += (const Quaternion &q);

	friend Quaternion operator + (const Quaternion &q1, const Quaternion& q2);
	friend Quaternion operator * (const Quaternion &q1, const Quaternion& q2);

	friend Quaternion operator * (const Quaternion &q, const Vector3& v);

	friend Quaternion operator * (float f, const Quaternion &q);
	friend Quaternion operator * (const Quaternion &q, float f);

	Quaternion& operator -();


	friend bool operator==(const Quaternion& q1, const Quaternion& q2);
	friend bool operator!=(const Quaternion& q1, const Quaternion& q2);

	void operator *= (float f);

	void computeMinimumRotation(const Vector3& rotateFrom, const Vector3& rotateTo);

	void normalize();
	float squaredLength() const;
	float length() const;
	void toMatrix(Matrix44 &) const;

	void toEulerAngles(Vector3 &euler) const;

	float& operator[] (unsigned int i) { return q[i]; }
};

float DotProduct(const Quaternion &q1, const Quaternion &q2);
Quaternion Qlerp(const Quaternion &q1, const Quaternion &q2, float t);
Quaternion Qslerp(const Quaternion &q1, const Quaternion &q2, float t);
Quaternion Qsquad(const Quaternion &q1, const Quaternion &q2, const Quaternion &a, const Quaternion &b, float t);
Quaternion Qsquad(const Quaternion &q1, const Quaternion &q2, const Quaternion &a, float t);
Quaternion Qspline(const Quaternion &q1, const Quaternion &q2, const Quaternion &q3);
Quaternion QslerpNoInvert(const Quaternion &q1, const Quaternion &q2, float t);
Quaternion Qexp(const Quaternion &q);
Quaternion Qlog(const Quaternion &q);
Quaternion SimpleRotation(const Vector3 &a, const Vector3 &b);

class BoundingBox
{
public:
	Vector3 center;
	Vector3 halfsize;
	BoundingBox() {}
	BoundingBox(Vector3 center, Vector3 halfsize) { this->center = center; this->halfsize = halfsize; };
};

//class to create 2D matrices of any size, useful for Images, Tilemaps, etc
template<typename T>
class Matrix
{
public:
	unsigned int width;
	unsigned int height;
	T* data;
	Matrix() { this->width = this->height = 0; data = NULL; }
	Matrix(int w, int h) { width = w; height = h; if (w&&h) { data = new T[w*h]; memset(data, 0, w*h * sizeof(T)); } else data = NULL; }
	Matrix(const Matrix<T>& c) { width = c.width; height = c.height; data = new T[width*height]; memcpy(data, c.data, width*height * sizeof(T)); }
	~Matrix() { if (data) delete[] data; }
	void set(int x, int y, T v) { assert(x >= 0 && x < (int)width && y >= 0 && y < (int)height); data[y*width + x] = v; }
	T get(int x, int y) const { assert(x >= 0 && x < (int)width && y >= 0 && y < (int)height); return data[y*width + x]; }
	T& get(int x, int y) { assert(x >= 0 && x < (int)width && y >= 0 && y < (int)height); return data[y*width + x]; }
	T& getMirrored(int x, int y) { x %= width; if (x < 0) x += width; y %= height; if (y < 0) y += height; return data[y*width + x]; }
	void fill(T v) { for (unsigned int x = 0; x < width; ++x) for (unsigned int y = 0; y < height; ++y) data[y * width + x] = v; }
	void operator = (const Matrix<T>& c) {
		if (data && c.width == width && c.height == height)	{ memcpy(data, c.data, width*height * sizeof(T)); return; }
		resize(c.width, c.height);
		if (data) memcpy(data, c.data, width*height * sizeof(T));
	}
	void resize(int w, int h)
	{
		if (data && width == w && height == h) return;
		if (data) delete[] data;
		width = w;
		height = h;
		if (width && height) data = new T[width*height];
		else data = NULL;
	}

	struct sMatrixHeader {
		unsigned int bom; //0xFFFF
		unsigned int w;
		unsigned int h;
		unsigned int tsize;
	};

	bool load(const char* filename)
	{
		FILE* f = fopen(filename, "rb");
		if (!f)
			return false;
		sMatrixHeader header;
		fread(&header, sizeof(sMatrixHeader), 1, f);
		if (header.bom != 0xFFFF)
			std::cerr << "Matrix file is not valid: " << filename << std::endl;
		else if (header.tsize != sizeof(T))
			std::cerr << "Matrix data type is not the same: " << filename << std::endl;
		else
		{
			if (data)
				delete data;
			width = header.w;
			height = header.h;
			data = new T[width * height];
			fread(data, width*height * sizeof(T), 1, f);
		}
		fclose(f);
		return true;
	}

	bool save(const char* filename)
	{
		if (width == 0 || height == 0)
		{
			std::cerr << "Cannot save Matrix with size 0" << std::endl;
			return false;
		}
		FILE* f = fopen(filename, "wb");
		if (!f)
			return false;
		sMatrixHeader header;
		header.bom = 0xFFFF;
		header.w = width;
		header.h = height;
		header.tsize = sizeof(T);
		fwrite(&header, sizeof(sMatrixHeader), 1, f);
		fwrite(data, sizeof(T), width * height, f );
		fclose(f);
		return true;
	}
};

//applies a transform to a AABB so it is 
BoundingBox transformBoundingBox(const Matrix44 m, const BoundingBox& box);

enum {
	CLIP_OUTSIDE = 0,
	CLIP_OVERLAP,
	CLIP_INSIDE
};

float signedDistanceToPlane(const Vector4& plane, const Vector3& point);
int planeBoxOverlap( const Vector4& plane, const Vector3& center, const Vector3& halfsize );
float ComputeSignedAngle( Vector2 a, Vector2 b);
Vector3 RayPlaneCollision( const Vector3& plane_pos, const Vector3& plane_normal, const Vector3& ray_origin, const Vector3& ray_dir );
float computeAngleDiff( float a, float b );

typedef Vector3 vec2;
typedef Vector3 vec3;
typedef Matrix44 mat4;
typedef Quaternion quat;

struct Segment
{
	Vector2 P0;
	Vector2 P1;
	Segment() {}
	Segment(Vector2 a, Vector2 b) { P0 = a; P1 = b; }
};

//useful to areas
class Area
{
public:
	union
	{
		struct { float x, y, w, h; };
		float v[4];
	};

	Area() { x = y = w = h = 0.0f; }
	Area(float x, float y, float w, float h) {
		this->x = x; this->y = y; this->w = w; this->h = h;
	}
	Area& set(float x, float y, float w, float h) { this->x = x; this->y = y; this->w = w; this->h = h; return *this; }
	bool inside(float px, float py) { return px >= x && py >= y && px < x + w && py < y + h; }
};

#endif
