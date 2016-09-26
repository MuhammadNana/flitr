/* Framework for Live Image Transformation (FLITr) 
 * Copyright (c) 2010 CSIR
 *
 * This file is part of FLITr.
 *
 * FLITr is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * FLITr is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FLITr. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <flitr/modules/geometry_overlays/arrow_overlay.h>

using namespace flitr;

ArrowOverlay::ArrowOverlay(double center_x, double center_y, double headWidth, double headHeight, double tailWidth, double tailLength, bool filled) :
    GeometryOverlay(),
    _TipX(center_x),
    _TipY(center_y),
    _HeadWidth(headWidth),
    _HeadHeight(headHeight),
    _TailWidth(tailWidth),
    _TailLength(tailLength)
{
    _Vertices = new osg::Vec3Array(7);

    makeArrow(filled);

    dirtyBound();

    setColour(osg::Vec4d(1,1,0,0.5));
}

ArrowOverlay::~ArrowOverlay()
{

}

void ArrowOverlay::makeArrow(bool filled)
{
    _Geode = new osg::Geode();
    _Geom = new osg::Geometry();

    updateArrow();
    
    _Geom->setVertexArray(_Vertices.get());
    
    if (!filled) {
        _DrawArray = new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 7);
    } else {
        _DrawArray = new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, 7);
    }
    _Geom->addPrimitiveSet(_DrawArray.get());
    _Geom->setUseDisplayList(false);
    _Geom->setCullingActive(false);

    _Geode->addDrawable(_Geom.get());
    _Geode->setCullingActive(false);

    _GeometryGroup->addChild(_Geode.get());
    _GeometryGroup->setCullingActive(false);
}

void ArrowOverlay::updateArrow()
{
    (*_Vertices)[0] = osg::Vec3d(_TipX                   , _TipY, 0);
    (*_Vertices)[1] = osg::Vec3d(_TipX + (_HeadWidth / 2), _TipY - (_HeadHeight), 0);
    (*_Vertices)[2] = osg::Vec3d(_TipX + (_TailWidth / 2), _TipY - (_HeadHeight), 0);
    (*_Vertices)[3] = osg::Vec3d(_TipX + (_TailWidth / 2), _TipY - (_HeadHeight + _TailLength), 0);
    (*_Vertices)[4] = osg::Vec3d(_TipX - (_TailWidth / 2), _TipY - (_HeadHeight + _TailLength), 0);
    (*_Vertices)[5] = osg::Vec3d(_TipX - (_TailWidth / 2), _TipY - (_HeadHeight), 0);
    (*_Vertices)[6] = osg::Vec3d(_TipX - (_HeadWidth / 2), _TipY - (_HeadHeight), 0);

    _Vertices->dirty();

    dirtyBound();
}

void ArrowOverlay::setTipPoint(double x, double y)
{
    _TipX = x;
    _TipY = y;
    updateArrow();
}

void ArrowOverlay::getTipPoint(double &x, double &y)
{
    x = _TipX;
    y = _TipY;
}

void ArrowOverlay::setHeadWidth(double width)
{
    _HeadWidth = width;
    updateArrow();
}
void ArrowOverlay::setHeadHeight(double height)
{
    _HeadHeight = height;
    updateArrow();
}
void ArrowOverlay::setTailWidth(double width)
{
    _TailWidth = width;
    updateArrow();
}
void ArrowOverlay::setTailLength(double length)
{
    _TailLength = length;
    updateArrow();
}

double ArrowOverlay::getHeadWidth() const
{
    return _HeadWidth;
}
double ArrowOverlay::getHeadHeight() const
{
    return _HeadHeight;
}
double ArrowOverlay::getTailWidth() const
{
    return _TailWidth;
}
double ArrowOverlay::getTailLength() const
{
    return _TailLength;
}

void ArrowOverlay::setName(const std::string &name)
{
    /* Set the name on the base class */
    GeometryOverlay::setName(name);

    /* Set the name of the other osg::Nodes */
    if (_Geom.valid() == true)
        _Geom->setName(name);

    if (_Geode.valid() == true)
        _Geode->setName(name);

    if (_DrawArray.valid() == true)
        _DrawArray->setName(name);
}

