// Copyright (C) 2014 von Karman Institute for Fluid Dynamics, Belgium

// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef ShockFitting_DiscrErrorNormL1_hh
#define ShockFitting_DiscrErrorNormL1_hh

//--------------------------------------------------------------------------//

#include <cmath>
#include "Framework/StateUpdater.hh"
#include "MathTools/Array2D.hh"

//--------------------------------------------------------------------------//

namespace ShockFitting {

//--------------------------------------------------------------------------//

/// This class define a DiscrErrorNormL1, whose task is to
/// compute the L1 norm of the discretization error over
/// the entire computational domain
/// the variables are already the primitive ones because they are converted in
/// Triangle2CFmesh

class DiscrErrorNormL1 : public StateUpdater {
public:

  /// Constructor
  /// @param objectName the concrete class name
  DiscrErrorNormL1(const std::string& objectName);

  /// Destructor
  virtual ~DiscrErrorNormL1();

  /// Set up this object before its first use
  virtual void setup();

  /// Unset up this object before its first use
  virtual void unsetup();

  /// compute the norm value
  virtual void update();

private: // functions

  /// assign the variables used in DiscrErrorNormL1 to MeshData pattern
  void setMeshData();

  /// assign the variables used in DiscrErrorNormL1 to PhysicsData pattern
  void setPhysicsData();

  /// set the starting pointers foer the array2D
  void setAddress();

  /// de-allocate the dynamic arrays
  void freeArray();

private: // data

  /// number of degrees of freedom
  unsigned* ndof;

  /// number of mesh points
  std::vector<unsigned>* npoin;

  /// mesh points state (assignable to MeshData)
  std::vector<double>* zroeVect;

  /// mesh points state belonging to the previous time step (assignable to MeshData)
  std::vector<double>* zroeOldVect;

  /// Array2D of the zroe values own to the current step
  Array2D <double>* zroe;

  /// Array2D of the zroe values own to the previous step
  Array2D <double>* zroeOld;

  /// value of the computed norm
  std::vector<double> normValue;
};

//--------------------------------------------------------------------------//

} // namespace ShockFitting

//--------------------------------------------------------------------------//

#endif // ShockFitting_DiscrErrorNormL1_hh