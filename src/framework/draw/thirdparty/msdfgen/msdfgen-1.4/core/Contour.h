
#pragma once

#include <vector>
#include "edge-segments.h"

namespace msdfgen {

/// A single closed contour of a shape.
class Contour {

public:
    /// The sequence of edges that make up the contour.
    std::vector<EdgeSegment> edges;

    /// Adds an edge to the contour.
    void addEdge(const EdgeSegment &edge);
#ifdef MSDFGEN_USE_CPP11
    void addEdge(EdgeSegment &&edge);
#endif
    /// Creates a new edge in the contour and returns its reference.
	EdgeSegment & addEdge();
    /// Computes the bounding box of the contour.
    void bounds(double &l, double &b, double &r, double &t) const;
    /// Computes the winding of the contour. Returns 1 if positive, -1 if negative.
    int winding() const;

};

}
