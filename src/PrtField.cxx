//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// Code developed by:
//  S.Larsson and J. Generowicz.

#include "G4SystemOfUnits.hh"
#include "G4AutoLock.hh"

#include "PrtField.h"

namespace {
G4Mutex myPrtFieldLock = G4MUTEX_INITIALIZER;
}

using namespace std;

PrtField::PrtField(const char *filename, double zOffset)
  : fZoffset(zOffset), invertX(false), invertY(false), invertZ(false) {

  double lenUnit = cm;
  double fieldUnit = gauss;
  G4cout << "\n-----------------------------------------------------------"
         << "\n      Magnetic field"
         << "\n-----------------------------------------------------------";

  G4cout << "\n ---> "
            "Reading the field grid from "
         << filename << " ... " << G4endl;

  //
  // This is a thread-local class and we have to avoid that all workers open the
  // file at the same time
  G4AutoLock lock(&myPrtFieldLock);

  ifstream file(filename); // Open the file for reading.

  if (!file.is_open()) {
    G4ExceptionDescription ed;
    ed << "Could not open input file " << filename << std::endl;
    G4Exception("PrtField::PrtField", "pugmag001", FatalException, ed);
  }

  char buffer[256];

  // Read table dimensions
  file >> nz >> ny >> nx; // Note dodgy order

  nx = ny;
  G4cout << "  [ Number of values x,y,z: " << nx << " " << ny << " " << nz << " ] " << G4endl;

  // Set up storage space for table
  xField.resize(nx);
  yField.resize(nx);
  zField.resize(nx);
  int ix, iy, iz;
  for (ix = 0; ix < nx; ix++) {
    xField[ix].resize(ny);
    yField[ix].resize(ny);
    zField[ix].resize(ny);
    for (iy = 0; iy < ny; iy++) {
      xField[ix][iy].resize(nz);
      yField[ix][iy].resize(nz);
      zField[ix][iy].resize(nz);
    }
  }

  // Ignore other header information
  // The first line whose second character is '0' is considered to
  // be the last line of the header.
  do {
    file.getline(buffer, 256);
  } while (buffer[1] != '0');

  // Read in the data
  double xval, yval, zval, bx, by, bz;

  for (iy = 0; iy < ny; iy++) {
    for (iz = 0; iz < nz; iz++) {

      file >> xval >> yval >> zval >> bx >> by >> bz;

      if (fabs(bx) < 0.01) bx = 0;
      if (fabs(by) < 0.01) by = 0;
      if (fabs(bz) < 0.01) bz = 0;

      for (ix = 0; ix < nx; ix++) {
        if (ix == 0 && iy == 0 && iz == 0) {
          minx = xval * lenUnit;
          miny = yval * lenUnit;
          minz = zval * lenUnit;
        }
        xField[ix][iy][iz] = bx * fieldUnit;
        yField[ix][iy][iz] = by * fieldUnit;
        zField[ix][iy][iz] = bz * fieldUnit;
      }
    }
  }
  file.close();

  lock.unlock();

  maxx = xval * lenUnit;
  maxy = yval * lenUnit;
  maxz = zval * lenUnit;

  G4cout << "\n ---> ... done reading " << G4endl;

  // G4cout << " Read values of field from file " << filename << G4endl;
  G4cout << " ---> assumed the order:  x, y, z, Bx, By, Bz "
         << "\n ---> Min values x,y,z: " << minx / cm << " " << miny / cm << " " << minz / cm
         << " cm "
         << "\n ---> Max values x,y,z: " << maxx / cm << " " << maxy / cm << " " << maxz / cm
         << " cm "
         << "\n ---> The field will be offset by " << zOffset / cm << " cm " << G4endl;

  // Should really check that the limits are not the wrong way around.
  if (maxx < minx) {
    swap(maxx, minx);
    invertX = true;
  }
  if (maxy < miny) {
    swap(maxy, miny);
    invertY = true;
  }
  if (maxz < minz) {
    swap(maxz, minz);
    invertZ = true;
  }
  G4cout << "\nAfter reordering if neccesary"
         << "\n ---> Min values x,y,z: " << minx / cm << " " << miny / cm << " " << minz / cm
         << " cm "
         << " \n ---> Max values x,y,z: " << maxx / cm << " " << maxy / cm << " " << maxz / cm
         << " cm ";

  dx = maxx - minx;
  dx = 1;
  dy = maxy - miny;
  dz = maxz - minz;
  G4cout << "\n ---> Dif values x,y,z (range): " << dx / cm << " " << dy / cm << " " << dz / cm
         << " cm in z "
         << "\n-----------------------------------------------------------" << G4endl;
}

void PrtField::GetFieldValue(const double point[4], double *Bfield) const {

  double x = fabs(point[0]);
  double y = fabs(point[1]);
  double z = point[2] + fZoffset;

  // Check that the point is within the defined region
  if (x >= miny && x <= maxy && y >= miny && y <= maxy && z >= minz && z <= maxz) {

    // Position of given point within region, normalized to the range
    // [0,1]
    double xfraction = (x - minx) / dy; 
    double yfraction = (y - miny) / dy;
    double zfraction = (z - minz) / dz;

    // Need addresses of these to pass to modf below.
    // modf uses its second argument as an OUTPUT argument.
    double xdindex, ydindex, zdindex;

    // Position of the point within the cuboid defined by the
    // nearest surrounding tabulated points
    double xlocal = (std::modf(xfraction * (nx - 1), &xdindex));
    double ylocal = (std::modf(yfraction * (ny - 1), &ydindex));
    double zlocal = (std::modf(zfraction * (nz - 1), &zdindex));

    // The indices of the nearest tabulated point whose coordinates
    // are all less than those of the given point
    int xindex = static_cast<int>(xdindex);
    int yindex = static_cast<int>(ydindex);
    int zindex = static_cast<int>(zdindex);

    // std::cout<<"-- "<<x<<" "<<y<<" "<<z<<std::endl;
    // std::cout << "00 " << xindex << " " << yindex << " " << zindex << std::endl;

    // std::cout<<"xField[xindex][yindex][zindex] "<<xField[xindex][yindex][zindex]<<" "<<yField[xindex][yindex][zindex]<<" "<< zField[xindex][yindex][zindex]<<std::endl;
    
    
    // // Full 3-dimensional version
    // Bfield[0] = xField[xindex][yindex][zindex] * (1 - xlocal) * (1 - ylocal) * (1 - zlocal) +
    //             xField[xindex][yindex][zindex + 1] * (1 - xlocal) * (1 - ylocal) * zlocal +
    //             xField[xindex][yindex + 1][zindex] * (1 - xlocal) * ylocal * (1 - zlocal) +
    //             xField[xindex][yindex + 1][zindex + 1] * (1 - xlocal) * ylocal * zlocal +
    //             xField[xindex + 1][yindex][zindex] * xlocal * (1 - ylocal) * (1 - zlocal) +
    //             xField[xindex + 1][yindex][zindex + 1] * xlocal * (1 - ylocal) * zlocal +
    //             xField[xindex + 1][yindex + 1][zindex] * xlocal * ylocal * (1 - zlocal) +
    //             xField[xindex + 1][yindex + 1][zindex + 1] * xlocal * ylocal * zlocal;
    // Bfield[1] = yField[xindex][yindex][zindex] * (1 - xlocal) * (1 - ylocal) * (1 - zlocal) +
    //             yField[xindex][yindex][zindex + 1] * (1 - xlocal) * (1 - ylocal) * zlocal +
    //             yField[xindex][yindex + 1][zindex] * (1 - xlocal) * ylocal * (1 - zlocal) +
    //             yField[xindex][yindex + 1][zindex + 1] * (1 - xlocal) * ylocal * zlocal +
    //             yField[xindex + 1][yindex][zindex] * xlocal * (1 - ylocal) * (1 - zlocal) +
    //             yField[xindex + 1][yindex][zindex + 1] * xlocal * (1 - ylocal) * zlocal +
    //             yField[xindex + 1][yindex + 1][zindex] * xlocal * ylocal * (1 - zlocal) +
    //             yField[xindex + 1][yindex + 1][zindex + 1] * xlocal * ylocal * zlocal;
    // Bfield[2] = zField[xindex][yindex][zindex] * (1 - xlocal) * (1 - ylocal) * (1 - zlocal) +
    //             zField[xindex][yindex][zindex + 1] * (1 - xlocal) * (1 - ylocal) * zlocal +
    //             zField[xindex][yindex + 1][zindex] * (1 - xlocal) * ylocal * (1 - zlocal) +
    //             zField[xindex][yindex + 1][zindex + 1] * (1 - xlocal) * ylocal * zlocal +
    //             zField[xindex + 1][yindex][zindex] * xlocal * (1 - ylocal) * (1 - zlocal) +
    //             zField[xindex + 1][yindex][zindex + 1] * xlocal * (1 - ylocal) * zlocal +
    //             zField[xindex + 1][yindex + 1][zindex] * xlocal * ylocal * (1 - zlocal) +
    //             zField[xindex + 1][yindex + 1][zindex + 1] * xlocal * ylocal * zlocal;

    Bfield[0] = xField[xindex][yindex][zindex];
    Bfield[1] = yField[xindex][yindex][zindex];
    Bfield[2] = zField[xindex][yindex][zindex];
    
    if (x != 0) Bfield[0] *= x / (point[0]);
    if (y != 0) Bfield[1] *= y / (point[1]);

    // std::cout << "Bfield[0] " << Bfield[0] << std::endl;

  } else {
    Bfield[0] = 0.0;
    Bfield[1] = 0.0;
    Bfield[2] = 0.0;
  }
}
