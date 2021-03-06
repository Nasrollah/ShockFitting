// Copyright (C) 2014 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef ShockFitting_MeshRestoring_hh
#define ShockFitting_MeshRestoring_hh

//--------------------------------------------------------------------------//

#include "Framework/CopyMaker.hh"
#include "MathTools/Array2D.hh"

//--------------------------------------------------------------------------//

namespace ShockFitting {

//--------------------------------------------------------------------------//

/// This class defines a MeshRestoring, whose task is to restore some of
/// the original arrays of the background mesh:
/// the nodal flag NODCOD
/// the boundary data structure BNDFAC
/// the boundary node pointer NODPTR

class MeshRestoring : public CopyMaker {
public:

  /// Constructor
  /// @param objectName the concrete class name
  MeshRestoring(const std::string& objectName);

  /// Destructor
  virtual ~MeshRestoring();

  /// Set up this object before its first use
  virtual void setup();

  /// Unset up object after its last use
  virtual void unsetup();

  /// restore mesh arrays
  virtual void copy();

private: // helper functions

  /// assign variables used in MeshRestoring to MeshData pattern
  void setMeshData();

  /// assign starting pointers for Array2D
  void setAddress();

  /// de-allocate the dynamic arrays
  void freeArray();

private: // data

  /// number of mesh points
  std::vector<unsigned>* npoin;

  /// number of boundary faces
  std::vector<unsigned>* nbfac;

  /// number of boundary points
  std::vector<unsigned>* nbpoin;

  /// mesh boundary faces (assignable to MeshData)
  std::vector<int>* bndfacVect;

  /// code characterizing mesh points
  std::vector <int>* nodcod;

  /// vector characterizing boundary nodes
  std::vector<int>* nodptrVect;

  /// backup vector of the code characterizing mesh points
  std::vector<int>* nodcodBackup;

  /// bndfac(0)(i-face) 1° endpoint of i-boundary face
  /// bndfac(1)(i-face) 2° endpoint of i-boundary face
  /// bndfac(2)(i-face) boundary marker of i-boundary face
  Array2D <int>* bndfac;

  /// backup array of boundary faces 
  Array2D <int>* bndfacBackup;

  /// nodptr(i-poin)(0) global code number
  /// nodptr(i-poin)(1) one of the edges it belongs to
  /// nodptr(i-poin)(2) other edge
  Array2D <int>* nodptr;

  /// backup array of array characterizing boundary nodes
  Array2D<int>* nodptrBackup;
};

//--------------------------------------------------------------------------//

} // namespace ShockFitting

//--------------------------------------------------------------------------//

#endif // ShockFitting_MeshRestoring_hh


