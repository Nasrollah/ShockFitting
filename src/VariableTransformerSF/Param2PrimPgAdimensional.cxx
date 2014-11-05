// Copyright (C) 2014 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "VariableTransformerSF/Param2PrimPgAdimensional.hh"
#include "Framework/Log.hh"
#include "SConfig/ObjectProvider.hh"

//--------------------------------------------------------------------------//

using namespace std;
using namespace SConfig;

//--------------------------------------------------------------------------//

namespace ShockFitting {

//--------------------------------------------------------------------------//

// this variable instantiation activates the self-registration mechanism
ObjectProvider<Param2PrimPgAdimensional, VariableTransformer>
 param2PrimPgAdimensionalProv("Param2PrimPgAdimensional");

//--------------------------------------------------------------------------//

Param2PrimPgAdimensional::Param2PrimPgAdimensional(const std::string& objectName) :
 Param2Prim(objectName)
{
}

//--------------------------------------------------------------------------//

Param2PrimPgAdimensional::~Param2PrimPgAdimensional()
{
}

//--------------------------------------------------------------------------//

void Param2PrimPgAdimensional::setup()
{
  LogToScreen(VERBOSE, "Param2PrimPgAdimensional::setup() => start \n");

  LogToScreen(VERBOSE, "Param2PrimPgAdimensional::setup() => end \n");
}

//--------------------------------------------------------------------------//

void Param2PrimPgAdimensional::unsetup()
{
  LogToScreen(VERBOSE, "Param2PrimPgAdimensional::unsetup() =>\n");
}

//--------------------------------------------------------------------------//

void Param2PrimPgAdimensional::transform()
{
  LogToScreen(INFO, "Param2PrimPgAdimensional::transform() \n");

  setPhysicsData();
  setMeshData();
  setAddress();

  (*rhoref) = (*pref) / (*Tref) / (*Rgas);

  for(unsigned IPOIN=0; IPOIN<npoin->at(1); IPOIN++) {
    rho = (*zroe)(0,IPOIN) * (*zroe)(0,IPOIN);
    kinetic = pow((*zroe)(2,IPOIN),2)+pow((*zroe)(3,IPOIN),2);
    kinetic = kinetic * 0.5;
    h = (*zroe)(1,IPOIN)/(*zroe)(0,IPOIN);
    help = ((*gam)-1)/(*gam);
    pres = help * (rho * h - kinetic);
    u = (*zroe)(2,IPOIN)/(*zroe)(0,IPOIN);
    v = (*zroe)(3,IPOIN)/(*zroe)(0,IPOIN);

    (*zroe)(0,IPOIN) = pres;
    (*zroe)(1,IPOIN) = u;
    (*zroe)(2,IPOIN) = v;
    (*zroe)(3,IPOIN) = pres/rho *
                         ((*uref) * (*uref) / (*Rgas) / (*Tref));
   }
}

//--------------------------------------------------------------------------//

} // namespace ShockFitting