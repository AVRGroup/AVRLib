/*  CCVT: ColourConVerT: simple library for converting colourspaces
    Copyright (C) 2002 Nemosoft Unv.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For questions, remarks, patches, etc. for this program, the author can be
    reached at nemosoft@smcc.demon.nl.
*/

/*
 $Log: ccvt.h,v $
 Revision 1.2  2006/12/06 00:37:23  retrakker
 - Added an updated CCVT version which hopefully will work as expected on
 x64 systems.

 Revision 1.1  2005/05/04 06:53:17  section314
 Added YUV420P-->RGB conversion

 Revision 1.10  2003/10/24 16:55:18  nemosoft
 removed erronous log messages

 Revision 1.9  2002/11/03 22:46:25  nemosoft
 Adding various RGB to RGB functions.
 Adding proper copyright header too.

 Revision 1.8  2002/04/14 01:00:27  nemosoft
 Finishing touches: adding const, adding libs for 'show'
*/


#ifndef CCVT_H
#define CCVT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Colour ConVerT: going from one colour space to another.
   ** NOTE: the set of available functions is far from complete! **

   Format descriptions:
   420i = "4:2:0 interlaced"
           YYYY UU YYYY UU   even lines
           YYYY VV YYYY VV   odd lines
           U/V data is subsampled by 2 both in horizontal
           and vertical directions, and intermixed with the Y values.

   420p = "4:2:0 planar"
           YYYYYYYY      N lines
           UUUU          N/2 lines
           VVVV          N/2 lines
           U/V is again subsampled, but all the Ys, Us and Vs are placed
           together in separate buffers. The buffers may be placed in
           one piece of contiguous memory though, with Y buffer first,
           followed by U, followed by V.

   yuyv = "4:2:2 interlaced"
           YUYV YUYV YUYV ...   N lines
           The U/V data is subsampled by 2 in horizontal direction only.

   bgr24 = 3 bytes per pixel, in the order Blue Green Red (whoever came up
           with that idea...)
   rgb24 = 3 bytes per pixel, in the order Red Green Blue (which is sensible)
   rgb32 = 4 bytes per pixel, in the order Red Green Blue Alpha, with
           Alpha really being a filler byte (0)
   bgr32 = last but not least, 4 bytes per pixel, in the order Blue Green Red
           Alpha, Alpha again a filler byte (0)
 */

#define PUSH_RGB24	1
#define PUSH_BGR24	2
#define PUSH_RGB32	3
#define PUSH_BGR32	4

/* This is a simplistic approach. */
static void ccvt_420p(int width, int height, const unsigned char *src, unsigned char *dst, int push)
{
	int line, col, linewidth;
	int y, u, v, yy, vr, ug, vg, ub;
	int r, g, b;
	const unsigned char *py, *pu, *pv;

	linewidth = width >> 1;
	py = src;
	pu = py + (width * height);
	pv = pu + (width * height) / 4;

	y = *py++;
	yy = y << 8;
	u = *pu - 128;
	ug =   88 * u;
	ub =  454 * u;
	v = *pv - 128;
	vg =  183 * v;
	vr =  359 * v;

	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col++) {
			r = (yy +      vr) >> 8;
			g = (yy - ug - vg) >> 8;
			b = (yy + ub     ) >> 8;

			if (r < 0)   r = 0;
			if (r > 255) r = 255;
			if (g < 0)   g = 0;
			if (g > 255) g = 255;
			if (b < 0)   b = 0;
			if (b > 255) b = 255;

			switch(push) {
			case PUSH_RGB24:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				break;

			case PUSH_BGR24:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				break;

			case PUSH_RGB32:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = 0;
				break;

			case PUSH_BGR32:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				*dst++ = 0;
				break;
			}

			y = *py++;
			yy = y << 8;
			if (col & 1) {
				pu++;
				pv++;

				u = *pu - 128;
				ug =   88 * u;
				ub =  454 * u;
				v = *pv - 128;
				vg =  183 * v;
				vr =  359 * v;
			}
		} /* ..for col */
		if ((line & 1) == 0) { // even line: rewind
			pu -= linewidth;
			pv -= linewidth;
		}
	} /* ..for line */
}


/* 4:2:0 YUV planar to RGB/BGR     */
void ccvt_420p_rgb24(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_RGB24);
}

void ccvt_420p_bgr24(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_BGR24);
}

void ccvt_420p_rgb32(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_RGB32);
}

void ccvt_420p_bgr32(int width, int height, const void *src, void *dst)
{
	ccvt_420p(width, height, (const unsigned char *)src, (unsigned char *)dst, PUSH_BGR32);
}
/* 4:2:2 YUYV interlaced to RGB/BGR */
void ccvt_yuyv_rgb32(int width, int height, const void *src, void *dst);
void ccvt_yuyv_rgb24(int width, int height, const void *src, void *dst);
void ccvt_yuyv_bgr32(int width, int height, const void *src, void *dst);

/* 4:2:2 YUYV interlaced to 4:2:0 YUV planar */
void ccvt_yuyv_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv);

/* RGB/BGR to 4:2:0 YUV interlaced */

/* RGB/BGR to 4:2:0 YUV planar     */
void ccvt_rgb24_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv);
void ccvt_bgr24_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv);

/* RGB/BGR to RGB/BGR */
void ccvt_bgr24_bgr32(int width, int height, const void *const src, void *const dst);
void ccvt_bgr24_rgb32(int width, int height, const void *const src, void *const dst);
void ccvt_bgr32_bgr24(int width, int height, const void *const src, void *const dst);
void ccvt_bgr32_rgb24(int width, int height, const void *const src, void *const dst);
void ccvt_rgb24_bgr32(int width, int height, const void *const src, void *const dst);
void ccvt_rgb24_rgb32(int width, int height, const void *const src, void *const dst);
void ccvt_rgb32_bgr24(int width, int height, const void *const src, void *const dst);
void ccvt_rgb32_rgb24(int width, int height, const void *const src, void *const dst);


#ifdef __cplusplus
}
#endif

#endif
