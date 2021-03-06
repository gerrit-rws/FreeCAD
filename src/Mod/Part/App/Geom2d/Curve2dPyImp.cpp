/***************************************************************************
 *   Copyright (c) 2016 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
# include <sstream>
# include <gp_Dir2d.hxx>
# include <gp_Vec2d.hxx>
# include <GCPnts_UniformAbscissa.hxx>
# include <GCPnts_UniformDeflection.hxx>
# include <GCPnts_TangentialDeflection.hxx>
# include <GCPnts_QuasiUniformAbscissa.hxx>
# include <GCPnts_QuasiUniformDeflection.hxx>
# include <GCPnts_AbscissaPoint.hxx>
# include <Geom2dAPI_InterCurveCurve.hxx>
# include <Geom2d_Geometry.hxx>
# include <Geom2d_Curve.hxx>
# include <Geom2dAdaptor_Curve.hxx>
# include <Geom2dLProp_CLProps2d.hxx>
# include <Precision.hxx>
# include <Geom2dAPI_ProjectPointOnCurve.hxx>
# include <Geom2dConvert_ApproxCurve.hxx>
# include <Standard_Failure.hxx>
# include <Standard_NullValue.hxx>
# include <ShapeConstruct_Curve.hxx>
# include <Geom2dAPI_ExtremaCurveCurve.hxx>
# include <BRepBuilderAPI_MakeEdge2d.hxx>
# include <BRepBuilderAPI_MakeEdge.hxx>
#endif

#include <Base/GeometryPyCXX.h>

#include <Mod/Part/App/Geometry2d.h>
#include <Mod/Part/App/GeometrySurfacePy.h>
#include <Mod/Part/App/Geom2d/BSplineCurve2dPy.h>
#include <Mod/Part/App/Geom2d/Curve2dPy.h>
#include <Mod/Part/App/Geom2d/Curve2dPy.cpp>

#include <Mod/Part/App/OCCError.h>
#include <Mod/Part/App/TopoShape.h>
#include <Mod/Part/App/TopoShapePy.h>
#include <Mod/Part/App/TopoShapeEdgePy.h>

namespace Part {
extern const Py::Object makeGeometryCurvePy(const Handle_Geom_Curve& c);
}

using namespace Part;

// returns a string which represents the object e.g. when printed in python
std::string Curve2dPy::representation(void) const
{
    return "<Curve2d object>";
}

PyObject *Curve2dPy::PyMake(struct _typeobject *, PyObject *, PyObject *)  // Python wrapper
{
    // never create such objects with the constructor
    PyErr_SetString(PyExc_RuntimeError,
        "You cannot create an instance of the abstract class 'Curve2d'.");
    return 0;
}

// constructor method
int Curve2dPy::PyInit(PyObject* /*args*/, PyObject* /*kwd*/)
{
    return 0;
}

PyObject* Curve2dPy::reverse(PyObject * /*args*/)
{
    try {
        Handle_Geom2d_Curve curve = Handle_Geom2d_Curve::DownCast(getGeom2dCurvePtr()->handle());
        curve->Reverse();
        Py_Return;
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

extern Py::Object shape2pyshape(const TopoDS_Shape &shape);

PyObject* Curve2dPy::toShape(PyObject *args)
{
    if (PyArg_ParseTuple(args, "")) {
        try {
            Handle_Geom2d_Curve curv = Handle_Geom2d_Curve::DownCast(getGeometry2dPtr()->handle());

            BRepBuilderAPI_MakeEdge2d mkBuilder(curv);
            TopoDS_Shape edge =  mkBuilder.Shape();
            return Py::new_reference_to(shape2pyshape(edge));
        }
        catch (Standard_Failure) {
            Handle_Standard_Failure e = Standard_Failure::Caught();
            PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
            return 0;
        }
    }

    PyErr_Clear();
    double u1, u2;
    if (PyArg_ParseTuple(args, "dd", &u1, &u2)) {
        try {
            Handle_Geom2d_Curve curv = Handle_Geom2d_Curve::DownCast(getGeometry2dPtr()->handle());

            BRepBuilderAPI_MakeEdge2d mkBuilder(curv, u1, u2);
            TopoDS_Shape edge =  mkBuilder.Shape();
            return Py::new_reference_to(shape2pyshape(edge));
        }
        catch (Standard_Failure) {
            Handle_Standard_Failure e = Standard_Failure::Caught();
            PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
            return 0;
        }
    }

    PyErr_Clear();
    PyObject* p;
    if (PyArg_ParseTuple(args, "O!", &(Part::GeometrySurfacePy::Type), &p)) {
        try {
            Handle_Geom_Surface surf = Handle_Geom_Surface::DownCast(
                        static_cast<GeometrySurfacePy*>(p)->getGeomSurfacePtr()->handle());
            Handle_Geom2d_Curve curv = Handle_Geom2d_Curve::DownCast(getGeometry2dPtr()->handle());

            BRepBuilderAPI_MakeEdge mkBuilder(curv, surf);
            TopoDS_Shape edge =  mkBuilder.Shape();
            return Py::new_reference_to(shape2pyshape(edge));
        }
        catch (Standard_Failure) {
            Handle_Standard_Failure e = Standard_Failure::Caught();
            PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
            return 0;
        }
    }

    PyErr_Clear();
    if (PyArg_ParseTuple(args, "O!dd", &(Part::GeometrySurfacePy::Type), &p, &u1, &u2)) {
        try {
            Handle_Geom_Surface surf = Handle_Geom_Surface::DownCast(
                        static_cast<GeometrySurfacePy*>(p)->getGeomSurfacePtr()->handle());
            Handle_Geom2d_Curve curv = Handle_Geom2d_Curve::DownCast(getGeometry2dPtr()->handle());

            BRepBuilderAPI_MakeEdge mkBuilder(curv, surf, u1, u2);
            TopoDS_Shape edge =  mkBuilder.Shape();
            return Py::new_reference_to(shape2pyshape(edge));
        }
        catch (Standard_Failure) {
            Handle_Standard_Failure e = Standard_Failure::Caught();
            PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
            return 0;
        }
    }

    PyErr_SetString(PyExc_TypeError, "empty parameter list, parameter range or surface expected");
    return 0;
}

PyObject* Curve2dPy::discretize(PyObject *args, PyObject *kwds)
{
    try {
        Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
        Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
        if (c.IsNull()) {
            PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
            return 0;
        }

        Geom2dAdaptor_Curve adapt(c);
        double first = adapt.FirstParameter();
        double last = adapt.LastParameter();

        // use Number kwds
        static char* kwds_numPoints[] = {"Number","First","Last",NULL};
        PyErr_Clear();
        int numPoints = -1;
        if (PyArg_ParseTupleAndKeywords(args, kwds, "i|dd", kwds_numPoints, &numPoints, &first, &last)) {
            GCPnts_UniformAbscissa discretizer;
            discretizer.Initialize (adapt, numPoints, first, last);

            if (discretizer.IsDone () && discretizer.NbPoints () > 0) {
                Py::List points;
                int nbPoints = discretizer.NbPoints ();

                Py::Module module("__FreeCADBase__");
                Py::Callable method(module.getAttr("Vector2d"));
                Py::Tuple arg(2);
                for (int i=1; i<=nbPoints; i++) {
                    gp_Pnt2d p = adapt.Value (discretizer.Parameter (i));
                    arg.setItem(0, Py::Float(p.X()));
                    arg.setItem(1, Py::Float(p.Y()));
                    points.append(method.apply(arg));
                }

                return Py::new_reference_to(points);
            }
            else {
                PyErr_SetString(PartExceptionOCCError, "Discretization of curve failed");
                return 0;
            }
        }

        // use Distance kwds
        static char* kwds_Distance[] = {"Distance","First","Last",NULL};
        PyErr_Clear();
        double distance = -1;
        if (PyArg_ParseTupleAndKeywords(args, kwds, "d|dd", kwds_Distance, &distance, &first, &last)) {
            GCPnts_UniformAbscissa discretizer;
            discretizer.Initialize (adapt, distance, first, last);

            if (discretizer.IsDone () && discretizer.NbPoints () > 0) {
                Py::List points;
                int nbPoints = discretizer.NbPoints ();

                Py::Module module("__FreeCADBase__");
                Py::Callable method(module.getAttr("Vector2d"));
                Py::Tuple arg(2);
                for (int i=1; i<=nbPoints; i++) {
                    gp_Pnt2d p = adapt.Value (discretizer.Parameter (i));
                    arg.setItem(0, Py::Float(p.X()));
                    arg.setItem(1, Py::Float(p.Y()));
                    points.append(method.apply(arg));
                }

                return Py::new_reference_to(points);
            }
            else {
                PyErr_SetString(PartExceptionOCCError, "Discretization of curve failed");
                return 0;
            }
        }

        // use Deflection kwds
        static char* kwds_Deflection[] = {"Deflection","First","Last",NULL};
        PyErr_Clear();
        double deflection;
        if (PyArg_ParseTupleAndKeywords(args, kwds, "d|dd", kwds_Deflection, &deflection, &first, &last)) {
            GCPnts_UniformDeflection discretizer(adapt, deflection, first, last);
            if (discretizer.IsDone () && discretizer.NbPoints () > 0) {
                Py::List points;
                int nbPoints = discretizer.NbPoints ();

                Py::Module module("__FreeCADBase__");
                Py::Callable method(module.getAttr("Vector2d"));
                Py::Tuple arg(2);
                for (int i=1; i<=nbPoints; i++) {
                    gp_Pnt p = discretizer.Value (i);
                    arg.setItem(0, Py::Float(p.X()));
                    arg.setItem(1, Py::Float(p.Y()));
                    points.append(method.apply(arg));
                }

                return Py::new_reference_to(points);
            }
            else {
                PyErr_SetString(PartExceptionOCCError, "Discretization of curve failed");
                return 0;
            }
        }

        // use TangentialDeflection kwds
        static char* kwds_TangentialDeflection[] = {"Angular","Curvature","First","Last","Minimum",NULL};
        PyErr_Clear();
        double angular;
        double curvature;
        int minimumPoints = 2;
        if (PyArg_ParseTupleAndKeywords(args, kwds, "dd|ddi", kwds_TangentialDeflection, &angular, &curvature, &first, &last, &minimumPoints)) {
            GCPnts_TangentialDeflection discretizer(adapt, first, last, angular, curvature, minimumPoints);
            if (discretizer.NbPoints () > 0) {
                Py::List points;
                int nbPoints = discretizer.NbPoints ();

                Py::Module module("__FreeCADBase__");
                Py::Callable method(module.getAttr("Vector2d"));
                Py::Tuple arg(2);
                for (int i=1; i<=nbPoints; i++) {
                    gp_Pnt p = discretizer.Value (i);
                    arg.setItem(0, Py::Float(p.X()));
                    arg.setItem(1, Py::Float(p.Y()));
                    points.append(method.apply(arg));
                }

                return Py::new_reference_to(points);
            }
            else {
                PyErr_SetString(PartExceptionOCCError, "Discretization of curve failed");
                return 0;
            }
        }

        // use QuasiNumber kwds
        static char* kwds_QuasiNumPoints[] = {"QuasiNumber","First","Last",NULL};
        PyErr_Clear();
        int quasiNumPoints;
        if (PyArg_ParseTupleAndKeywords(args, kwds, "i|dd", kwds_QuasiNumPoints, &quasiNumPoints, &first, &last)) {
            GCPnts_QuasiUniformAbscissa discretizer(adapt, quasiNumPoints, first, last);
            if (discretizer.NbPoints () > 0) {
                Py::List points;
                int nbPoints = discretizer.NbPoints ();

                Py::Module module("__FreeCADBase__");
                Py::Callable method(module.getAttr("Vector2d"));
                Py::Tuple arg(2);
                for (int i=1; i<=nbPoints; i++) {
                    gp_Pnt2d p = adapt.Value (discretizer.Parameter (i));
                    arg.setItem(0, Py::Float(p.X()));
                    arg.setItem(1, Py::Float(p.Y()));
                    points.append(method.apply(arg));
                }

                return Py::new_reference_to(points);
            }
            else {
                PyErr_SetString(PartExceptionOCCError, "Discretization of curve failed");
                return 0;
            }
        }

        // use QuasiDeflection kwds
        static char* kwds_QuasiDeflection[] = {"QuasiDeflection","First","Last",NULL};
        PyErr_Clear();
        double quasiDeflection;
        if (PyArg_ParseTupleAndKeywords(args, kwds, "d|dd", kwds_QuasiDeflection, &quasiDeflection, &first, &last)) {
            GCPnts_QuasiUniformDeflection discretizer(adapt, quasiDeflection, first, last);
            if (discretizer.NbPoints () > 0) {
                Py::List points;
                int nbPoints = discretizer.NbPoints ();

                Py::Module module("__FreeCADBase__");
                Py::Callable method(module.getAttr("Vector2d"));
                Py::Tuple arg(2);
                for (int i=1; i<=nbPoints; i++) {
                    gp_Pnt p = discretizer.Value (i);
                    arg.setItem(0, Py::Float(p.X()));
                    arg.setItem(1, Py::Float(p.Y()));
                    points.append(method.apply(arg));
                }

                return Py::new_reference_to(points);
            }
            else {
                PyErr_SetString(PartExceptionOCCError, "Discretization of curve failed");
                return 0;
            }
        }
    }
    catch (const Base::Exception& e) {
        PyErr_SetString(PartExceptionOCCError, e.what());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError,"Wrong arguments");
    return 0;
}

PyObject* Curve2dPy::length(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double u=c->FirstParameter();
            double v=c->LastParameter();
            double t=Precision::Confusion();
            if (!PyArg_ParseTuple(args, "|ddd", &u,&v,&t))
                return 0;
            Geom2dAdaptor_Curve adapt(c);
            double len = GCPnts_AbscissaPoint::Length(adapt,u,v,t);
            return PyFloat_FromDouble(len);
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::parameterAtDistance(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double abscissa;
            double u = 0;
            if (!PyArg_ParseTuple(args, "d|d", &abscissa,&u))
                return 0;
            Geom2dAdaptor_Curve adapt(c);
            GCPnts_AbscissaPoint abscissaPoint(adapt,abscissa,u);
            double parm = abscissaPoint.Parameter();
            return PyFloat_FromDouble(parm);
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::value(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double u;
            if (!PyArg_ParseTuple(args, "d", &u))
                return 0;
            gp_Pnt2d p = c->Value(u);

            Py::Module module("__FreeCADBase__");
            Py::Callable method(module.getAttr("Vector2d"));
            Py::Tuple arg(2);
            arg.setItem(0, Py::Float(p.X()));
            arg.setItem(1, Py::Float(p.Y()));
            return Py::new_reference_to(method.apply(arg));
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::tangent(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double u;
            if (!PyArg_ParseTuple(args, "d", &u))
                return 0;
            gp_Dir2d dir;
            Geom2dLProp_CLProps2d prop(c,u,2,Precision::Confusion());
            if (prop.IsTangentDefined()) {
                prop.Tangent(dir);
            }

            Py::Module module("__FreeCADBase__");
            Py::Callable method(module.getAttr("Vector2d"));
            Py::Tuple arg(2);
            arg.setItem(0, Py::Float(dir.X()));
            arg.setItem(1, Py::Float(dir.Y()));
            return Py::new_reference_to(method.apply(arg));
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::normal(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double u;
            if (!PyArg_ParseTuple(args, "d", &u))
                return 0;
            gp_Dir2d dir;
            Geom2dLProp_CLProps2d prop(c,u,2,Precision::Confusion());
            prop.Normal(dir);

            Py::Module module("__FreeCADBase__");
            Py::Callable method(module.getAttr("Vector2d"));
            Py::Tuple arg(2);
            arg.setItem(0, Py::Float(dir.X()));
            arg.setItem(1, Py::Float(dir.Y()));
            return Py::new_reference_to(method.apply(arg));
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::curvature(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double u;
            if (!PyArg_ParseTuple(args, "d", &u))
                return 0;
            Geom2dLProp_CLProps2d prop(c,u,2,Precision::Confusion());
            double C = prop.Curvature();
            return Py::new_reference_to(Py::Float(C));
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::centerOfCurvature(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double u;
            if (!PyArg_ParseTuple(args, "d", &u))
                return 0;
            Geom2dLProp_CLProps2d prop(c,u,2,Precision::Confusion());
            gp_Pnt2d pnt ;
            prop.CentreOfCurvature(pnt);

            Py::Module module("__FreeCADBase__");
            Py::Callable method(module.getAttr("Vector2d"));
            Py::Tuple arg(2);
            arg.setItem(0, Py::Float(pnt.X()));
            arg.setItem(1, Py::Float(pnt.Y()));
            return Py::new_reference_to(method.apply(arg));
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::parameter(PyObject *args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            PyObject *p;
            if (!PyArg_ParseTuple(args, "O!", Base::Vector2dPy::type_object(), &p))
                return 0;
            Base::Vector2d v = Py::Vector2d(p).getCxxObject()->value();
            gp_Pnt2d pnt(v.x,v.y);
            Geom2dAPI_ProjectPointOnCurve ppc(pnt, c);
            double val = ppc.LowerDistanceParameter();
            return Py::new_reference_to(Py::Float(val));
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::toBSpline(PyObject * args)
{
    Handle_Geom2d_Geometry g = getGeometry2dPtr()->handle();
    Handle_Geom2d_Curve c = Handle_Geom2d_Curve::DownCast(g);
    try {
        if (!c.IsNull()) {
            double u,v;
            u=c->FirstParameter();
            v=c->LastParameter();
            if (!PyArg_ParseTuple(args, "|dd", &u,&v))
                return 0;
            ShapeConstruct_Curve scc;
            Handle_Geom2d_BSplineCurve spline = scc.ConvertToBSpline(c, u, v, Precision::Confusion());
            if (spline.IsNull())
                Standard_NullValue::Raise("Conversion to B-Spline failed");
            return new BSplineCurve2dPy(new Geom2dBSplineCurve(spline));
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PartExceptionOCCError, "Geometry is not a curve");
    return 0;
}

PyObject* Curve2dPy::approximateBSpline(PyObject *args)
{
    double tolerance;
    int maxSegment, maxDegree;
    char* order = "C2";
    if (!PyArg_ParseTuple(args, "dii|s", &tolerance, &maxSegment, &maxDegree, &order))
        return 0;

    GeomAbs_Shape absShape;
    std::string str = order;
    if (str == "C0")
        absShape = GeomAbs_C0;
    else if (str == "G1")
        absShape = GeomAbs_G1;
    else if (str == "C1")
        absShape = GeomAbs_C1;
    else if (str == "G2")
        absShape = GeomAbs_G2;
    else if (str == "C2")
        absShape = GeomAbs_C2;
    else if (str == "C3")
        absShape = GeomAbs_C3;
    else if (str == "CN")
        absShape = GeomAbs_CN;
    else
        absShape = GeomAbs_C2;

    try {
        Handle_Geom2d_Curve self = Handle_Geom2d_Curve::DownCast(getGeometry2dPtr()->handle());
        Geom2dConvert_ApproxCurve approx(self, tolerance, absShape, maxSegment, maxDegree);
        if (approx.IsDone()) {
            return new BSplineCurve2dPy(new Geom2dBSplineCurve(approx.Curve()));
        }
        else if (approx.HasResult()) {
            std::stringstream str;
            str << "Maximum error (" << approx.MaxError() << ") is outside tolerance";
            PyErr_SetString(PyExc_RuntimeError, str.str().c_str());
            return 0;
        }
        else {
            PyErr_SetString(PyExc_RuntimeError, "Approximation of curve failed");
            return 0;
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PartExceptionOCCError, e->GetMessageString());
        return 0;
    }
}

Py::String Curve2dPy::getContinuity(void) const
{
    GeomAbs_Shape c = Handle_Geom2d_Curve::DownCast
        (getGeometry2dPtr()->handle())->Continuity();
    std::string str;
    switch (c) {
    case GeomAbs_C0:
        str = "C0";
        break;
    case GeomAbs_G1:
        str = "G1";
        break;
    case GeomAbs_C1:
        str = "C1";
        break;
    case GeomAbs_G2:
        str = "G2";
        break;
    case GeomAbs_C2:
        str = "C2";
        break;
    case GeomAbs_C3:
        str = "C3";
        break;
    case GeomAbs_CN:
        str = "CN";
        break;
    default:
        str = "Unknown";
        break;
    }
    return Py::String(str);
}

Py::Boolean Curve2dPy::getClosed(void) const
{
    return Py::Boolean(Handle_Geom2d_Curve::DownCast
        (getGeometry2dPtr()->handle())->IsClosed() ? true : false);
}

Py::Boolean Curve2dPy::getPeriodic(void) const
{
    return Py::Boolean(Handle_Geom2d_Curve::DownCast
        (getGeometry2dPtr()->handle())->IsPeriodic() ? true : false);
}

Py::Float Curve2dPy::getFirstParameter(void) const
{
    return Py::Float(Handle_Geom2d_Curve::DownCast
        (getGeometry2dPtr()->handle())->FirstParameter());
}

Py::Float Curve2dPy::getLastParameter(void) const
{
    return Py::Float(Handle_Geom2d_Curve::DownCast
        (getGeometry2dPtr()->handle())->LastParameter());
}

PyObject *Curve2dPy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int Curve2dPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}

PyObject* Curve2dPy::intersectCC(PyObject *args)
{
    Handle_Geom2d_Curve curve1 = Handle_Geom2d_Curve::DownCast(getGeometry2dPtr()->handle());
    try {
        if (!curve1.IsNull()) {
            PyObject *p;
            double prec = Precision::Confusion();
            if (!PyArg_ParseTuple(args, "O!|d", &(Part::Curve2dPy::Type), &p, &prec))
                return 0;
            Handle_Geom2d_Curve curve2 = Handle_Geom2d_Curve::DownCast(static_cast<Geometry2dPy*>(p)->getGeometry2dPtr()->handle());
            Geom2dAPI_ExtremaCurveCurve intersector(curve1, curve2,
                                                    curve1->FirstParameter(),
                                                    curve1->LastParameter(),
                                                    curve2->FirstParameter(),
                                                    curve2->LastParameter());
            if (intersector.LowerDistance() > Precision::Confusion()) {
                // No intersection
                return Py::new_reference_to(Py::List());
            }

            Py::List points;
            Py::Module module("__FreeCADBase__");
            Py::Callable method(module.getAttr("Vector2d"));
            Py::Tuple arg(2);
            for (int i = 1; i <= intersector.NbExtrema(); i++) {
                if (intersector.Distance(i) > Precision::Confusion())
                    continue;
                gp_Pnt2d p1, p2;
                intersector.Points(i, p1, p2);

                arg.setItem(0, Py::Float(p1.X()));
                arg.setItem(1, Py::Float(p1.Y()));
                points.append(method.apply(arg));
            }

            return Py::new_reference_to(points);
        }
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        PyErr_SetString(PyExc_Exception, e->GetMessageString());
        return 0;
    }

    PyErr_SetString(PyExc_Exception, "Geometry is not a curve");
    return 0;
}
