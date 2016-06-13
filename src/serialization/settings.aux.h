#include "../Enums.h"
#include <QPoint>
#include <QSize>
#include <QMatrix>

#define DEFINE_INLINE_GETTER(x) const decltype(x) &get_##x() const{ return this->x; } 
#define DEFINE_INLINE_NONCONST_GETTER(x) decltype(x) &get_##x(){ return this->x; } 
#define DEFINE_INLINE_SETTER(x) void set_##x(const decltype(x) &v){ this->x = v; }
#define DEFINE_ENUM_INLINE_GETTER(t, x) t get_##x() const{ return (t)this->x; } 
#define DEFINE_ENUM_INLINE_SETTER(t, x) void set_##x(const t &v){ this->x = (decltype(this->x))v; }
#define DEFINE_INLINE_SETTER_GETTER(x) DEFINE_INLINE_GETTER(x) DEFINE_INLINE_SETTER(x)
#define DEFINE_ENUM_INLINE_SETTER_GETTER(t, x) DEFINE_ENUM_INLINE_GETTER(t, x) DEFINE_ENUM_INLINE_SETTER(t, x)
