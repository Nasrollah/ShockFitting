// Copyright (C) 2014 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "VariableTransformerSF/Param2PrimTCneqDimensional.hh"
#include "Framework/ChemicalConsts.hh"
#include "Framework/Log.hh"
#include "SConfig/ObjectProvider.hh"
#include "VariableTransformerSF/ComputeTv.hh"

//--------------------------------------------------------------------------//

using namespace std;
using namespace SConfig;

//--------------------------------------------------------------------------//

namespace ShockFitting {

//--------------------------------------------------------------------------//

// this variable instantiation activates the self-registration mechanism
ObjectProvider<Param2PrimTCneqDimensional, VariableTransformer>
 param2PrimTCneqDimensionalProv("Param2PrimTCneqDimensional");

//--------------------------------------------------------------------------//

Param2PrimTCneqDimensional::Param2PrimTCneqDimensional
                            (const std::string& objectName) :
 Param2Prim(objectName)
{
}

//--------------------------------------------------------------------------//

Param2PrimTCneqDimensional::~Param2PrimTCneqDimensional()
{
}

//--------------------------------------------------------------------------//

void Param2PrimTCneqDimensional::setup()
{
  LogToScreen(VERBOSE,"Param2PrimTCneqDimensional::setup() => start\n");

  LogToScreen(VERBOSE,"Param2PrimTCneqDimensional::setup() => end\n");
}

//--------------------------------------------------------------------------//

void Param2PrimTCneqDimensional::unsetup()
{
  LogToScreen(VERBOSE,"Param2PrimTCneqDimensional::unsetup()\n");
}

//--------------------------------------------------------------------------//

void Param2PrimTCneqDimensional::transform()
{
  LogToScreen(INFO,"Param2PrimTCneqDimensional::transform()\n");

  setPhysicsData();
  setMeshData();
  setAddress();

  cout.precision(10);

  for(unsigned IPOIN=0; IPOIN<npoin->at(1); IPOIN++) {
    // zrho and rho
    double sqrtr = 0;
    for (unsigned ISP=0; ISP<(*nsp); ISP++) {
     sqrtr = sqrtr + (*zroe)(ISP,IPOIN);  }
    rho = pow(sqrtr,2);

    // rho-i and alpha-i
    alpha.resize(*nsp);
    rhos.resize(*nsp);
    for (unsigned ISP=0; ISP<(*nsp); ISP++) {
     alpha.at(ISP) = (*zroe)(ISP,IPOIN)/sqrtr;
     rhos.at(ISP) = (*zroe)(ISP,IPOIN) * sqrtr * (*rhoref); }     

    // u, v, h, ev
    u.resize(2);
    h = (*zroe)((*ie),IPOIN)/sqrtr * pow((*uref),2);
    u.at(0) = (*zroe)((*ix),IPOIN)/sqrtr * (*uref);
    u.at(1) = (*zroe)((*iy),IPOIN)/sqrtr * (*uref);
    ev = (*zroe)((*iev),IPOIN)/sqrtr * pow((*uref),2);

    // kinetic energy
    kinetic = pow(u.at(0),2)+pow(u.at(1),2);
    kinetic = kinetic * 0.5;

    // formation enthalpy
    double hftot = 0;
    for(unsigned ISP=0; ISP<(*nsp); ISP++) {
     // pow((*uref),2) is used because of the hf dimensionalization
     // done by the ReferenceInfo object
     // in ReferenceInfo hf->at(ISP) = hf->at(ISP)/((*uref) * (*uref));
     hftot = hftot + alpha.at(ISP) * hf->at(ISP)*pow((*uref),2);
    }

    // roto-traslation enthalpy
    double htr = h - kinetic - hftot - ev;

    // roto-translation specific heat
    double Cp = 0;
    for(unsigned ISP=0; ISP<(*nsp); ISP++) {
     Cp = Cp + alpha.at(ISP) * gams->at(ISP)/(gams->at(ISP)-1)
          * ChemicalConsts::Rgp()/mm->at(ISP);
    }

    // temperature
    T.resize(2);

    T.at(0) = htr/Cp;

    // vibrational temperature
    ComputeTv cTv;
    if((*nmol)==1) {
     for(unsigned ISP=0; ISP<(*nsp); ISP++) {
      if(typemol->at(ISP)=="B") { IM = ISP;}
     }
     T.at(1) = T.at(0)*pow(10,-6);

     cTv.callComputeTv(ev,alpha,T);
     T = cTv.getT();
    }

    else if (*nmol>1) { T.at(1)=T.at(0);
                        cTv.callComputeTv(ev,alpha,T);
                        T=cTv.getT(); }

    else { cout << "ConverterSF::Parm2Prim4TCneq::error => NMOL = 0\n";
           exit(1); }

    for(unsigned ISP=0; ISP<(*nsp); ISP++) {
     (*zroe)(ISP,IPOIN)=rhos.at(ISP);
    }

   (*zroe)((*ie),IPOIN) = u.at(0);
   (*zroe)((*ix),IPOIN) = u.at(1);
   (*zroe)((*iy),IPOIN) = T.at(0);
   (*zroe)((*iev),IPOIN) = T.at(1);
   

   (*XY)(0,IPOIN) = (*XY)(0,IPOIN) * (*Lref);
   (*XY)(1,IPOIN) = (*XY)(1,IPOIN) * (*Lref);
  }
}  

//--------------------------------------------------------------------------//

} // namespace ShockFitting
