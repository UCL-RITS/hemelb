//
// Copyright (C) University College London, 2007-2012, all rights reserved.
//
// This file is part of HemeLB and is CONFIDENTIAL. You may not work
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
//

#include "PolyDataGenerator.h"

#include "Neighbours.h"
#include "Site.h"
#include "InconsistentFluidnessError.h"

#include "Debug.h"

#include "io/formats/geometry.h"

#include "vtkPolyDataAlgorithm.h"
#include "vtkOBBTree.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkIdList.h"
#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkMatrix4x4.h"
#include "Block.h"
#include "vtkXMLPolyDataWriter.h"

#include <iostream>

using namespace hemelb::io::formats;

PolyDataGenerator::PolyDataGenerator() :
		GeometryGenerator(), ClippedSurface(NULL) {
	this->Locator = vtkOBBTree::New();
	//this->Locator->SetNumberOfCellsPerNode(32); // the default
	this->Locator->SetTolerance(1e-9);
	this->hitPoints = vtkPoints::New();
	this->hitCellIds = vtkIdList::New();
	std::ifstream     in;
	//const char* infile = "working_cylinder_clipped.off";
	const char* infile = "broken_cylinder_clipped.off";
        in.open(infile);
	std::istream* p_in = &in;
	static Polyhedron P;
	(*p_in) >> P;
	if (!*p_in) {
	  std::cerr << "error: cannot open file"<< std::endl;
	  exit( 1);
	}
	this->numberofvoxels = 0;
	this->ClippedCGALSurface = &P;
	this->AABBtree = new Tree(this->ClippedCGALSurface->facets_begin(),this->ClippedCGALSurface->facets_end());
	this->inside_with_ray = new PointInside(P);
       	//cout << (*this->inside_with_ray)(p1) << endl;
}

PolyDataGenerator::~PolyDataGenerator() {
	this->Locator->Delete();
	this->hitPoints->Delete();
	this->hitCellIds->Delete();
	delete this->AABBtree;
	delete this->inside_with_ray;
	//delete this->ClippedCGALSurface;
}

void PolyDataGenerator::ComputeBounds(double bounds[6]) const {
	this->ClippedSurface->GetBounds(bounds);
}

void PolyDataGenerator::PreExecute(void) {
	// Build our locator.
	this->Locator->SetDataSet(this->ClippedSurface);
	this->Locator->BuildLocator();

	/*
	 * Get the scalars associated with the surface polygons. The scalars hold
	 * the index of the Iolet which they represent, -1 meaning they aren't an
	 * Iolet.
	 */
	this->IoletIdArray = vtkIntArray::SafeDownCast(
			this->ClippedSurface->GetCellData()->GetScalars());
	if (this->IoletIdArray == NULL) {
		throw GenerationErrorMessage(
				"Error getting Iolet ID array from clipped surface");
	}
	
}

/*
 * Given a site with known fluidness, examine the links to not-yet-visited
 * neighbouring sites. If the neighbours have unknown fluidness, set that.
 *
 * Since we wish to examine each link only once, this will set link properties
 * from neighbour => site as well as site => neighbour
 *
 */
void PolyDataGenerator::ClassifySite(Site& site) {
  for (LaterNeighbourIterator neighIt = site.begin(); neighIt != site.end();
			++neighIt) {
		Site& neigh = *neighIt;
		unsigned int iNeigh = neighIt.GetNeighbourIndex();
		vtkIdType nHits;
		PointCGAL p1(site.Position.x,site.Position.y,site.Position.z);
		PointCGAL p2(neigh.Position.x,neigh.Position.y,neigh.Position.z);
		//bool inside1 = (*this->inside_with_ray)(p1);
		bool inside2 = (*this->inside_with_ray)(p2);
		if (!neigh.IsFluidKnown) {
			// Neighbour unknown, must always intersect
			nHits = this->ComputeIntersections(site, neigh);
			neigh.IsFluid = inside2;
			
			if (neigh.IsFluid)
			  this->numberofvoxels++;
			  neigh.CreateLinksVector();

			neigh.IsFluidKnown = true;
		} else {
			// We know the fluidness of neigh, maybe don't need to intersect
			// Only in the case of difference must we intersect.
			nHits = this->ComputeIntersections(site, neigh);
		}
		
		// Four cases: fluid-fluid, solid-solid, fluid-solid and solid-fluid.
		// Will handle the last two together.
		if (site.IsFluid == neigh.IsFluid) {
			if (site.IsFluid) {
				// Fluid-fluid, must set CUT_NONE for both
				site.Links[iNeigh].Type = geometry::CUT_NONE;
				neigh.Links[Neighbours::inverses[iNeigh]].Type =
						geometry::CUT_NONE;
			} else {
				// solid-solid, nothing to do.
			}
		} else {
			// They differ, figure out which is fluid and which is solid.
			Site* fluid;
			Site* solid;

			// Index of the solid site from the fluid site.
			int iSolid;
			// Index of the point in this->hitPoints we're considering as the
			// hit of interest (i.e. the one closest to the fluid site).
			int iHit;

			if (site.IsFluid) {
				fluid = &site;
				solid = &neigh;
				iSolid = iNeigh;   
				iHit = 0;
			} else {
				fluid = &neigh;
				solid = &site;
				iSolid = Neighbours::inverses[iNeigh];
				iHit = nHits - 1;
			}
			
			Vector hitPoint;
			this->hitPoints->GetPoint(iHit, &hitPoint[0]);
			// needs logic to pick the right point of the list. Depending on the fluid 
			Vector hitPointCGAL = Vector(this->HitPointsCGAL[0].x(),this->HitPointsCGAL[0].y(),this->HitPointsCGAL[0].z());
			//cout << hitPointCGAL << " "<< hitPoint << endl;
			LinkData& link = fluid->Links[iSolid];

			//if (nHits != 1) {
			//  cout << "hits: " << nHits << " and " << nHitsCGAL <<endl;
			//  cout << hitPoint << endl;
			  //cout << this->HitPointCGAL << endl;
			//}
			// This is set in any solid case
			float distanceInVoxels =
					(hitPointCGAL - fluid->Position).GetMagnitude();
			// The distance is in voxels but must be output as a fraction of
			// the lattice vector. Scale it.
			link.Distance = distanceInVoxels / Neighbours::norms[iSolid];

			// The index of the cell in the vtkPolyData that was hit
			int hitCellId = this->hitCellIds->GetId(iHit);
			// The value associated with that cell, which identifies what was hit.
			int ioletId = this->IoletIdArray->GetValue(hitCellId);

			if (ioletId < 0) {
				// -1 => we hit a wall
				link.Type = geometry::CUT_WALL;
			} else {
				// We hit an inlet or outlet
				Iolet* iolet = this->Iolets[ioletId];
				if (iolet->IsInlet) {
					link.Type = geometry::CUT_INLET;
				} else {
					link.Type = geometry::CUT_OUTLET;
				}
				// Set the Id
				link.IoletId = iolet->Id;
			}

			// If this link intersected the wall, store the normal of the cell we hit and the distance to it.
			if (link.Type == geometry::CUT_WALL) {
				double* normal =
						this->Locator->GetDataSet()->GetCellData()->GetNormals()->GetTuple3(
								hitCellId);
				link.WallNormalAtWallCut = Vector(normal[0], normal[1],
						normal[2]);
				link.DistanceInVoxels = distanceInVoxels;
			}
		}
		} 
	
	// If there's enough information available, an approximation of the wall normal will be computed for this fluid site.
	this->ComputeAveragedNormal(site);
	//cout << "site " <<site.Position << endl;
	//cout << "Voxels "<< this->numberofvoxels << endl;
}


int PolyDataGenerator::ComputeIntersections(Site& from, Site& to) {
	this->Locator->IntersectWithLine(&from.Position[0], &to.Position[0],
			this->hitPoints, this->hitCellIds);
	int hitpoints = this->hitPoints->GetNumberOfPoints();
	PointCGAL p1(from.Position[0], from.Position[1], from.Position[2]);
	PointCGAL p2(to.Position[0], to.Position[1], to.Position[2]);
	PointCGAL p3;
	PointCGAL hitpoint;
	SegmentCGAL s1;
	SegmentCGAL segment_query(p1,p2);
	std::vector<Object_and_primitive_id> CGALintersections;
	nHitsCGAL = this->AABBtree->number_of_intersected_primitives(segment_query);
	this->HitPointsCGAL.clear();
	this->AABBtree->all_intersections(segment_query,std::back_inserter(CGALintersections));
	double mindist = 1e100;
	double temp;
	
	if (nHitsCGAL) {
	    for (std::vector<Object_and_primitive_id>::iterator i = CGALintersections.begin(); i != CGALintersections.end(); ++i) {
	      CGAL::Object object = i->first;
	      if(CGAL::assign(p3,object)){
		this->HitPointsCGAL.push_back(p3);
	      }
	      else if (CGAL::assign(s1,object)){
		cout << "line todo" << endl;
		//temp = CGAL::squared_distance(p2,s1);
	      }
	    } 
	  }
	return hitpoints;
}


// Function to be called on intersecting leaf nodes of the two OBB trees.
// Final void pointer is a pointer to an int, namely the count of the number
// of intersections found so far, which is incremented.
int IntersectingLeafCounter(vtkOBBNode* polyNode, vtkOBBNode* cubeNode,
		vtkMatrix4x4* transform, void *ptr_to_intersection_count) {
	int &intersection_count = *static_cast<int*>(ptr_to_intersection_count);
	intersection_count++;
}

int PolyDataGenerator::BlockInsideOrOutsideSurface(const Block &block) {
	// Create an OBB tree for the block
	vtkOBBTree *blockSlightlyLargerOBBTree = block.CreateOBBTreeModel(1.0);

	// Count the number of domain OBB leaf nodes that intersect the single
	// node created for the block.
	int intersection_count = 0;
	Locator->IntersectWithOBBTree(blockSlightlyLargerOBBTree, NULL,
			IntersectingLeafCounter, static_cast<void*>(&intersection_count));
	// Delete the underlying polydata
	blockSlightlyLargerOBBTree->GetDataSet()->Delete();
	// And the OBBTree itself
	blockSlightlyLargerOBBTree->Delete();

	if (intersection_count == 0) {
		// either entirely inside or entirely outside
		double middlePosition[3];
		middlePosition[0] = block.Middle().Position[0];
		middlePosition[1] = block.Middle().Position[1];
		middlePosition[2] = block.Middle().Position[2];
		return Locator->InsideOrOutside(middlePosition);
	}
	return 0;
}
