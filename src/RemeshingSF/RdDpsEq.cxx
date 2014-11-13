// Copyright (C) 2014 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>
#include "RemeshingSF/RdDpsEq.hh"
#include "SConfig/ObjectProvider.hh"
#include "Framework/Remeshing.hh"
#include "Framework/Log.hh"
#include "Framework/PhysicsData.hh"
#include "Framework/PhysicsInfo.hh"
#include "Framework/MeshData.hh"

//--------------------------------------------------------------------------//

using namespace std;
using namespace SConfig;

//--------------------------------------------------------------------------//

namespace ShockFitting {

//--------------------------------------------------------------------------//

// this variable instantiation activates the self-registration mechanism
ObjectProvider<RdDpsEq, Remeshing> rdDpsEqProv("RdDpsEq");

//--------------------------------------------------------------------------//

RdDpsEq::RdDpsEq(const std::string& objectName) :
  Remeshing(objectName)
{
}

//--------------------------------------------------------------------------//

RdDpsEq::~RdDpsEq()
{
  delete ZRoeShu; delete ZRoeShd;
}

//--------------------------------------------------------------------------//

void RdDpsEq::setup()
{
  LogToScreen(VERBOSE, "RdDpsEq::setup() => start\n");

  setPhysicsData();
  setMeshData();
  logfile.Open(getClassName());

  LogToScreen(VERBOSE, "RdDpsEq::setup() => end\n");
}

//--------------------------------------------------------------------------//

void RdDpsEq::unsetup()
{
  LogToScreen(VERBOSE, "RdDpsEq::unsetup()\n");

  logfile.Close();
}

//--------------------------------------------------------------------------//

void RdDpsEq::remesh()
{
  LogToScreen(INFO, "RdDpsEq::remesh()\n");

  // assign start pointers of Array2D and 3D
  setAddress();

  // resize vectors and arrays
  setSize();

  for (unsigned I=0; I<(*nShocks); I++){

   // compute length of shock edge
   computeShEdgeLength(I);

   // compute number of shock points of redistribution
   // and check the new number of shock points
   computeNbDistrShPoints(I);

   // compute distribution step
   computeDistrStep(I);

   // interpolation of new shock points
   // set the first and the last point
   newPointsInterp(I);

   // computation of internal points
   computeInterPoints(I);

   // rewrite the state arrays and indices
   rewriteValues(I);
  }
}

//--------------------------------------------------------------------------//

void RdDpsEq::computeShEdgeLength(unsigned index)
{
  ISH=index;
  unsigned ish = ISH+1;
  logfile("Shock n. :", ish,"\n");
  Sh_ABSC.at(0) = 0;
  for (unsigned IV=0; IV<nShockEdges->at(ISH); IV++) {
   unsigned I = IV+1;
   Sh_Edge_length = 0;
   for (unsigned K=0; K<PhysicsInfo::getnbDim(); K++) {
    Sh_Edge_length = Sh_Edge_length +
                     pow(((*XYSh)(K,IV,ISH)-(*XYSh)(K,I,ISH)),2);
   }
   Sh_Edge_length = sqrt(Sh_Edge_length);
   Sh_ABSC.at(I) = Sh_ABSC.at(I-1) + Sh_Edge_length;
  }

  logfile("Number of points old distribution:", nShockPoints->at(ISH),"\n");
}

//--------------------------------------------------------------------------//

void RdDpsEq::computeNbDistrShPoints(unsigned index)
{
  ISH=index;
  nShockPoints_new = Sh_ABSC.at(nShockPoints->at(ISH)-1) /
                     (MeshData::getInstance().getDXCELL())+1;

  if (nShockPoints_new > PhysicsInfo::getnbShPointsMax()) {
   cout << "RdDpsEq::error => Too many shock points! Increase NPSHMAX in input.case\n";
   exit(1);}
}

//--------------------------------------------------------------------------//

void RdDpsEq::computeDistrStep(unsigned index)
{
  ISH=index;
  double dx = Sh_ABSC.at(nShockPoints->at(ISH)-1)/(nShockPoints_new-1);
  Sh_ABSC_New.at(0) = 0;
  for (unsigned IV=0; IV<nShockPoints_new; IV++) {
   unsigned I = IV+1;
   Sh_ABSC_New.at(I) = Sh_ABSC_New.at(I-1) + dx;

  }
  logfile ("Number of points new distribution:",nShockPoints_new, "\n");
}

//--------------------------------------------------------------------------//

void RdDpsEq::newPointsInterp(unsigned index)
{
  ISH = index;

   for (unsigned K=0; K<PhysicsInfo::getnbDim(); K++) {
    XYSh_New(K,0) = (*XYSh)(K,0,ISH);
   }

   for (unsigned K=0; K<(*ndof); K++) {
    ZRoeShu_New(K,0) = (*ZRoeShu)(K,0,ISH);
    ZRoeShd_New(K,0) = (*ZRoeShd)(K,0,ISH);
   }

   for (unsigned K=0; K<PhysicsInfo::getnbDim(); K++) {
    XYSh_New(K,nShockPoints_new-1) = (*XYSh)(K,nShockPoints->at(ISH)-1,ISH);

   }

   for (unsigned K=0; K<(*ndof); K++) {
    ZRoeShu_New(K,nShockPoints_new-1) =
                              (*ZRoeShu)(K,nShockPoints->at(ISH)-1,ISH);
    ZRoeShd_New(K,nShockPoints_new-1) =
                              (*ZRoeShd)(K,nShockPoints->at(ISH)-1,ISH);
   }
}

//--------------------------------------------------------------------------//

void RdDpsEq::computeInterPoints(unsigned index)
{
  double ds, dsj, alpha, beta;
  unsigned j1, i1;
  ISH=index;
  for(unsigned i=1; i<nShockPoints_new-1; i++) {
   for (unsigned j=1; j<nShockPoints->at(ISH); j++) {
    if ( Sh_ABSC_New.at(i) >= Sh_ABSC.at(j-1) && 
         Sh_ABSC_New.at(i) <  Sh_ABSC.at(j) ) {
     ds = Sh_ABSC.at(j)-Sh_ABSC.at(j-1);
     dsj = Sh_ABSC_New.at(i)-Sh_ABSC.at(j-1);
     alpha = dsj/ds;
     beta = 1-alpha;
     j1 = j-1; i1 = i-1;
     logfile("node=",j1,"\n");
     logfile("node_new=",i1,"\n");
     for (unsigned K=0; K<PhysicsInfo::getnbDim(); K++) {
      XYSh_New(K,i) = beta * (*XYSh)(K,j1,ISH) + alpha * (*XYSh)(K,j,ISH);
     }
     for (unsigned K=0; K<(*ndof); K++) {
      ZRoeShu_New(K,i) = beta * (*ZRoeShu)(K,j1,ISH) + 
                         alpha * (*ZRoeShu)(K,j,ISH);
      ZRoeShd_New(K,i) = beta * (*ZRoeShd)(K,j1,ISH) + 
                         alpha * (*ZRoeShd)(K,j,ISH); 
      logfile("ZROESHd(",K,"=",(*ZRoeShd)(K,j1,ISH), "\n");
     } // K
    } // if
   } // j
  } // i
}

//--------------------------------------------------------------------------//

void RdDpsEq::rewriteValues(unsigned index)
{
  ISH=index;
  logfile("new shock point coordinates");
  for (unsigned I=0; I<nShockPoints_new; I++) {
   for(unsigned K=0; K<PhysicsInfo::getnbDim(); K++) {
    (*XYSh)(K,I,ISH)= XYSh_New(K,I);
    //logfile(K)
    logfile((*XYSh)(K,I,ISH), " ");
   }
   logfile("\n");
   for(unsigned IV=0; IV<(*ndof); IV++) {
    logfile(ZRoeShd_New(IV,I), " ");
    (*ZRoeShu)(IV,I,ISH) = ZRoeShu_New(IV,I);
    (*ZRoeShd)(IV,I,ISH) = ZRoeShd_New(IV,I);
   }
   logfile("\n");
  }
  nShockPoints->at(ISH)=nShockPoints_new;
  nShockEdges->at(ISH) = nShockPoints_new-1;  
}

//--------------------------------------------------------------------------//

void RdDpsEq::setAddress()
{
  unsigned start;
  start = npoin->at(0)*(*ndof);
  ZRoeShu = new Array3D <double> ((*ndof),
                                  PhysicsInfo::getnbShPointsMax(),
                                  PhysicsInfo::getnbShMax(),
                                  &zroeVect->at(start));
  start = npoin->at(0) * (*ndof) +
          PhysicsInfo::getnbShPointsMax() *
          PhysicsInfo::getnbShMax() *
          (*ndof);
  ZRoeShd = new Array3D <double> (PhysicsInfo::getnbDofMax(),
                                  PhysicsInfo::getnbShPointsMax(),
                                  PhysicsInfo::getnbShMax(),
                                  &zroeVect->at(start));
}

//--------------------------------------------------------------------------//

void RdDpsEq::setSize()
{
  XYSh_New.resize(PhysicsInfo::getnbDim(),
                  PhysicsInfo::getnbShPointsMax() );
  ZRoeShu_New.resize((*ndof) ,
                     PhysicsInfo::getnbShPointsMax() );
  ZRoeShd_New.resize((*ndof),
                     PhysicsInfo::getnbShPointsMax() );
  Sh_ABSC.resize(PhysicsInfo::getnbShPointsMax());
  Sh_ABSC_New.resize(PhysicsInfo::getnbShPointsMax());
}

//--------------------------------------------------------------------------//

void RdDpsEq::setPhysicsData()
{
  ndof = PhysicsData::getInstance().getData <unsigned> ("NDOF");
  nShocks = PhysicsData::getInstance().getData <unsigned> ("nShocks"); 
  nShockPoints = PhysicsData::getInstance().getData <vector <unsigned> > ("nShockPoints");
  nShockEdges = PhysicsData::getInstance().getData <vector <unsigned> > ("nShockEdges");
  XYSh = PhysicsData::getInstance().getData <Array3D <double> > ("XYSH");
}

//--------------------------------------------------------------------------//

void RdDpsEq::setMeshData()
{
  zroeVect = MeshData::getInstance().getData <vector <double> >("ZROE");
  npoin = MeshData::getInstance().getData <vector<unsigned> > ("NPOIN");
}

//--------------------------------------------------------------------------//

} // namespace ShockFitting
