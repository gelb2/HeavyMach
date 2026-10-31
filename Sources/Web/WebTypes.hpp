#pragma once
// Web platform geometry. Mirrors the CoreGraphics semantics used by Sources/iOS
// (CGFloat is double on 64-bit Apple targets) so translated code keeps the same
// arithmetic, rectangle containment and intersection rules.
#include <algorithm>
#include <cmath>
#include <limits>

namespace hmweb {
struct Point {double x=0,y=0;};
struct Size {double width=0,height=0;};
struct Rect {Point origin;Size size;};

inline Point makePoint(double x,double y){return {x,y};}
inline Rect makeRect(double x,double y,double w,double h){return {{x,y},{w,h}};}
inline Rect nullRect(){const double inf=std::numeric_limits<double>::infinity();return {{inf,inf},{0,0}};}
inline bool isNull(const Rect& r){return std::isinf(r.origin.x)||std::isinf(r.origin.y);}
// CGRect accessors standardise negative sizes.
inline double minX(const Rect& r){return std::min(r.origin.x,r.origin.x+r.size.width);}
inline double maxX(const Rect& r){return std::max(r.origin.x,r.origin.x+r.size.width);}
inline double minY(const Rect& r){return std::min(r.origin.y,r.origin.y+r.size.height);}
inline double maxY(const Rect& r){return std::max(r.origin.y,r.origin.y+r.size.height);}
inline double midX(const Rect& r){return (minX(r)+maxX(r))/2;}
inline double midY(const Rect& r){return (minY(r)+maxY(r))/2;}
inline double width(const Rect& r){return std::fabs(r.size.width);}
inline double height(const Rect& r){return std::fabs(r.size.height);}
inline bool isEmpty(const Rect& r){return isNull(r)||r.size.width==0||r.size.height==0;}
inline Rect offset(const Rect& r,double dx,double dy){if(isNull(r))return r;return makeRect(r.origin.x+dx,r.origin.y+dy,r.size.width,r.size.height);}
inline Rect inset(const Rect& r,double dx,double dy){
    if(isNull(r))return r;
    Rect s=makeRect(minX(r)+dx,minY(r)+dy,width(r)-2*dx,height(r)-2*dy);
    if(s.size.width<0||s.size.height<0)return nullRect();
    return s;
}
// CGRectContainsPoint: includes min, excludes max.
inline bool contains(const Rect& r,Point p){
    return !isEmpty(r)&&p.x>=minX(r)&&p.x<maxX(r)&&p.y>=minY(r)&&p.y<maxY(r);
}
inline Rect intersection(const Rect& a,const Rect& b){
    if(isNull(a)||isNull(b))return nullRect();
    const double x0=std::max(minX(a),minX(b)),x1=std::min(maxX(a),maxX(b));
    const double y0=std::max(minY(a),minY(b)),y1=std::min(maxY(a),maxY(b));
    if(x1<x0||y1<y0)return nullRect();
    return makeRect(x0,y0,x1-x0,y1-y0);
}
inline bool equal(const Rect& a,const Rect& b){
    return a.origin.x==b.origin.x&&a.origin.y==b.origin.y&&a.size.width==b.size.width&&a.size.height==b.size.height;
}
}
