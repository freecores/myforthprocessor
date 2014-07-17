/*
 * @(#)TX.cpp	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "TX.h"

// almost completely unoptimized
void TX::concat(const TX& rhs) {
    if (rhs.isIdentity()) {
	return;
    }
    if (isIdentity()) {
	set(rhs);
	return;
    }
    jdouble T00 = rhs.m00;
    jdouble T10 = rhs.m10;
    jdouble T01 = rhs.m01;
    jdouble T11 = rhs.m11;
    jdouble T02 = rhs.m02;
    jdouble T12 = rhs.m12;

    jdouble M0 = m00;
    jdouble M1 = m01;
    m00  = T00 * M0 + T10 * M1;
    m01  = T01 * M0 + T11 * M1;
    m02 += T02 * M0 + T12 * M1;

    M0 = m10;
    M1 = m11;
    m10  = T00 * M0 + T10 * M1;
    m11  = T01 * M0 + T11 * M1;
    m12 += T02 * M0 + T12 * M1;

    initType();
}

jboolean TX::invert() {
    if (!isIdentity()) {
	jdouble det = m00 * m11 - m01 * m10;
	if (det < .000001 && det > -.000001) { // good enough for us
            return JNI_FALSE;
	}

	//	fprintf(stderr, "mat: %g %g %g %g %g %g\n", m00, m10, m01, m11, m02, m12);
	//	fprintf(stderr, "det: %g\n", det);

	jdouble x00 = m11 / det;
	jdouble x10 = -m10 / det;
	jdouble x01 = -m01 / det;
	jdouble x11 = m00 / det;
	jdouble x02 = (m01 * m12 - m11 * m02) / det;
	jdouble x12 = (m10 * m02 - m00 * m12) / det;
	    
	m00 = x00;
	m10 = x10;
	m01 = x01;
	m11 = x11;
	m02 = x02;
	m12 = x12;

	//	fprintf(stderr, "new mat: %g %g %g %g %g %g\n", m00, m10, m01, m11, m02, m12);

	initType();
    }
    return JNI_TRUE;
}

void TX::transform(jfloat points[], int count, int type) const {
  jfloat x;

  if (type == TYPE_IDENTITY) {
      return;
  }

  for (jfloat *p = points, *e = points + count * 2; p != e; p += 2) {
      //      fprintf(stderr, "%g %g --> ", p[0], p[1]);

    switch (type) {
    case TYPE_IDENTITY: // should not occur
      break;
    case MASK_TRANS:
      p[0] += m02;
      p[1] += m12;
      break;
    case MASK_SCALE:
      p[0] *= m00;
      p[1] *= m11;
      break;
    case MASK_SCALE | MASK_TRANS:
      p[0] = p[0] * m00 + m02;
      p[1] = p[1] * m11 + m12;
      break;
    case MASK_SHEAR:
      x = p[0];
      p[0] += p[1] * m01;
      p[1] += x    * m10;
      break;
    case MASK_SHEAR | MASK_TRANS:
      x = p[0];
      p[0] += p[1] * m01 + m02;
      p[1] += x    * m10 + m12;
      break;
    case MASK_SHEAR | MASK_SCALE:
      x = p[0];
      p[0] = x * m00 + p[1] * m01;
      p[1] = x * m10 + p[1] * m11;
      break;
    default:
    case MASK_SHEAR | MASK_SCALE | MASK_TRANS:
      x = p[0];
      p[0] = x * m00 + p[1] * m01 + m02;
      p[1] = x * m10 + p[1] * m11 + m12;
      break;
    }

    //    fprintf(stderr, "%g %g\n", p[0], p[1]);
  }
}

void TX::transform(jfloat &x, jfloat &y, int type) const {
  jfloat tx;
  switch (type) {
  case TYPE_IDENTITY:
    break;
  case MASK_TRANS:
    x += m02;
    y += m12;
    break;
  case MASK_SCALE:
    x *= m00;
    y *= m11;
    break;
  case MASK_SCALE | MASK_TRANS:
    x = x * m00 + m02;
    y = y * m11 + m12;
    break;
  case MASK_SHEAR:
    tx = x;
    x +=  y * m01;
    y += tx * m10;
    break;
  case MASK_SHEAR | MASK_TRANS:
    tx = x;
    x +=  y * m01 + m02;
    y += tx * m10 + m12;
    break;
  case MASK_SHEAR | MASK_SCALE:
    tx = x;
    x =  x * m00 + y * m01;
    y = tx * m10 + y * m11;
    break;
  default:
  case MASK_SHEAR | MASK_SCALE | MASK_TRANS:
    tx = x;
    x =  x * m00 + y * m01 + m02;
    y = tx * m10 + y * m11 + m12;
    break;
  }
}
