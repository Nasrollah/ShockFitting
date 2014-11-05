// Copyright (C) 2014 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef ShockFitting_Triangle2CFmesh_hh
#define ShockFitting_Triangle2CFmesh_hh

//----------------------------------------------------------------------------//

#include <iomanip>
#include <string>
#include "Framework/Converter.hh"
#include "MathTools/Array2D.hh"
#include "VariableTransformerSF/Param2Prim.hh"

#define PAIR_TYPE(a) SConfig::StringT<SConfig::SharedPtr<a> >

//----------------------------------------------------------------------------//

namespace ShockFitting {

//----------------------------------------------------------------------------//

/// This class defines Triangle2CFmesh, whose task is to make conversion
/// from Triangle mesh generator format to CFmesh files format.
/// The new values read on triangles files are stored in new arrays to not
/// overwrite the old mesh status.
/// These new values are pushed back at the end of the arrays of the old mesh
/// in the fortran version these new arrays are referred to index "1"
/// (ex: ZROE(1))

class Triangle2CFmesh : public Converter {
public:

  /// Constructor
  /// @param objectName the concrete class name
  Triangle2CFmesh(const std::string& objectName);

  /// Destructor
  virtual ~Triangle2CFmesh();

  /// Set up this object before its first use
  virtual void setup();

  /// Unset up this object after its last use
  virtual void unsetup();

  /// convert files
  virtual void convert();

protected: // functions

  /// Configures the options for this object.
  /// To be extended by derived classes.
  /// @param args is the ConfigArgs with the arguments to be parsed.
  virtual void configure(SConfig::OptionMap& cmap, const std::string& prefix);

private: // helper functions

  /// read triangle mesh generator format file
  void readTriangleFmt();

  /// write CFmesh format file
  void writeCFmeshFmt();

  /// assign variables used in ReadTrianleFmt to MeshData pattern
  void setMeshData();

  /// assign variables used in ReadTrianleFmt to PhysicsData pattern
  void setPhysicsData();

private: // data

  /// space dimension
  unsigned* ndim;

  /// number of degrees of freedom
  unsigned* ndof;

  /// max number of degrees of freedom
  unsigned* ndofmax;

  /// number of vertices
  unsigned* nvt;

  /// max number of shock
  unsigned* nshmax;
  
  /// max number of points for each shock
  unsigned* npshmax;
  
  /// max number of edges for each shock
  unsigned* neshmax;

  /// number of mesh points
  std::vector<unsigned>* npoin;

  /// number of mesh elements
  std::vector<unsigned>* nelem;

  /// number of boundary faces
  std::vector<unsigned>* nbfac;

  /// mesh points status
  std::vector <double>* zroeVect;

  /// mesh points coordinates
  std::vector <double>* coorVect;

  /// vector characterizing nodes elements (assignable to MeshData)
  std::vector<int>* celnodVect;

  /// vector characterizing mesh elements (assignable to MeshData)
  std::vector<int>* celcelVect;

  /// vector characterizing boundary faces (assignable to MeshData)
  std::vector<int>* bndfacVect;

  /// name of the current file
  std::vector<std::string>* fname;

  /// mesh points coordinates (in array storing)
  Array2D <double>* XY;

  /// mesh points status (in array storing)
  Array2D <double>* zroe;

  /// celnod(0)(i-elem) 1° node of i-element
  /// celnod(1)(i-elem) 2° node of i-element
  /// celnod(2)(i-elem) 3° node of i-element
  Array2D <int>* celnod;

  /// celcel(0)(i-elem) 1° neighbor of i-element
  /// celcel(1)(i-elem) 2° neighbor of i-element
  /// celcel(2)(i-elem) 3° neighbor of i-element
  Array2D <int>* celcel;

  /// bndfac(0)(i-face) 1° endpoint of i-boundary face
  /// bndfac(1)(i-face) 2° endpoint of i-boundary face
  /// bndfac(2)(i-face) boundary marker of i-boundary face
  Array2D <int>* bndfac;

  /// dummy variables to store array dimension
  unsigned totsize;

  /// dummy variable to store array starting pointer
  unsigned start;

  /// dummy vector used to store boundary edges colours
  std::vector<int> ICLR;

  /// command object transforming variables
  PAIR_TYPE(VariableTransformer) m_param2prim;

};

//----------------------------------------------------------------------------//

} // namespace ShockFitting

//----------------------------------------------------------------------------//

#endif // ShockFitting_Triangle2CFmesh_hh