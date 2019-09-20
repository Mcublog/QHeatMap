/*====================================================================
 * Filename:      heatmapper.cpp
 * Version:       
 * Author:        Dianchun Huang <simpleotter23@gmail.com>
 * 
 * Copyright (c) 2013 Dianchun Huang (simpleotter23@gmail.com)
 * 
 * Created at:    Thu May 23 23:39:03 2013
 * Modified at:   Tue Jul  9 23:45:21 2013
 * Description:   
 *==================================================================*/
#include "heatmapper.h"
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QRadialGradient>
#include <QDebug>
#include "gradientpalette.h"

/*
 * Constructor
 * @param image Image for displaying output
 * @param radius The radius determines the size of the radial gradient
 * @param opacity Opacity, the range of values is[0, 100]
 */
HeatMapper::HeatMapper(QImage *image, GradientPalette *palette,
					   int radius, int opacity)
	: radius_(radius), opacity_(opacity), max_(1)
{
	Q_ASSERT(image);
	Q_ASSERT(palette);

	palette_ = palette;
	
	mainCanvas_ = image;
	alphaCanvas_ = new QImage(image->size(), QImage::Format_ARGB32);
	Q_ASSERT(alphaCanvas_);
	alphaCanvas_->fill(QColor(0, 0, 0, 0));

	width_ = image->width();
	height_ = image->height();
	data_.resize(width_ * height_);
	data_.fill(0);
}

HeatMapper::~HeatMapper()
{}

/*
 * Increase the number of hits at the specified coordinate point
 * @param x abscissa
 * @param y ordinate
 * @param delta increased number of times
 * @return returns the number of hits after the update
 */
int HeatMapper::increase(int x, int y, int delta)
{
	int index = (y - 1) * width_ + (x - 1);
	data_[index] += delta;
	return data_[index];
}

/*
 * Save image to file
 * @param fname filename
 */
void HeatMapper::save(const QString &fname)
{
	
}

/*
 * Add a data point
 * @param x abscissa
 * @param y ordinate
 */
void HeatMapper::addPoint(int x, int y)
{
	if (x <= 0 || y <= 0 || x > width_ || y > height_)
		return;

	int count = increase(x, y);

	if (max_ < count) {
		max_ = count;
		redraw();
		return;
	}

	drawAlpha(x, y, count);
}

/*
 * Redraw the entire image based on the maximum number of hits, this method produces a afterglow effect
 */
void HeatMapper::redraw()
{
	QColor color(0, 0, 0, 0);
	alphaCanvas_->fill(color);
	mainCanvas_->fill(color);

	int size = data_.size();
	for (int i = 0; i < size; ++i) {
		if (0 == data_[i])
			continue;
		drawAlpha(i % width_ + 1, i / width_ + 1, data_[i], false);
	}
	colorize();
}

/*
 * Set the palette
 * @param palette palette object pointer
 */
void HeatMapper::setPalette(GradientPalette *palette)
{
	Q_ASSERT(palette);
	
	if (palette)
		palette_ = palette;
}

/*
 * Get the number of hits at the specified point
 */
int HeatMapper::getCount(int x, int y)
{
	if (x < 0 || y < 0)
		return 0;
	return data_[(y - 1) * width_ + (x - 1)];
}

/*
 * Draw a transparent radial gradient
 * @param x abscissa
 * @param y ordinate
 * @param count hits
 * @param colorize_now Whether to call the shading method
 */
void HeatMapper::drawAlpha(int x, int y, int count, bool colorize_now)
{
	int alpha = int(qreal(count * 1.0 / max_)*255);
	QRadialGradient gradient(x, y, radius_);
	gradient.setColorAt(0, QColor(0, 0, 0, alpha));
	gradient.setColorAt(1, QColor(0, 0, 0, 0));
	
	QPainter painter(alphaCanvas_);
	painter.setPen(Qt::NoPen);
	painter.setBrush(gradient);
	painter.drawEllipse(QPoint(x, y), radius_, radius_);

	if (colorize_now)
		colorize(x, y);
}

/*
 * Overloaded method, coloring
 */
void HeatMapper::colorize()
{
	colorize(0, 0, width_, height_);
}

/*
 * overloaded method, coloring
 * @param x abscissa
 * @param y ordinate
 * @param subArea transparent radial gradient area
 */
void HeatMapper::colorize(int x, int y)
{
	int left = x - radius_;
	int top = y - radius_;
	int right = x + radius_;
	int bottom = y + radius_;
	QColor color;

	if (left < 0)
		left = 0;

	if (top < 0)
		top = 0;

	if (right > width_)
		right = width_;

	if (bottom > height_)
		bottom = height_;

	colorize(left, top, right, bottom);
}

/*
 * overloaded function, the actual shading operation in this method
 * @param left upper left horizontal axis
 * @param top upper left ordinate
 * @param right lower right corner abscissa
 * @param bottom lower right ordinate
 */
void HeatMapper::colorize(int left, int top, int right, int bottom)
{
	int alpha = 0;
	int finalAlpha = 0;
	QColor color;
	for (int i = left; i < right; ++i) {
		for (int j = top; j < bottom; ++j) {
			alpha = qAlpha(alphaCanvas_->pixel(i, j));
			if (!alpha)
				continue;
			finalAlpha = (alpha < opacity_ ? alpha : opacity_);
			color = palette_->getColorAt(alpha);
			mainCanvas_->setPixel(i, j, qRgba(color.red(),
											  color.green(),
											  color.blue(),
											  finalAlpha));
		}
	}
}
