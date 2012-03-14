#ifndef KSPAINTER_H
#define KSPAINTER_H

#include <qpainter.h>
#include <qobject.h>

class KSPainter 
{
public:
  static void initOptColours();
  static void colourDrawText(QPainter *p, int startx, int starty, char *str, int length = -1);
  static const int startspecial;
  static const int maxcolour;
  static const int optcolours;
  static QColor num2colour[16+8];
  static int colour2num(const QColor &colour);
  
  static const QColor brown;
  static const QColor orange;
  static const QColor lightCyan;
  static const QColor lightBlue;
  static const QColor pink;
};

#endif
