/**
 * @file    LibavoidRouting.h
 * @author  uru
 *
 * @section LICENSE
 *
 * Copyright (c) 2013, 2022 Kiel University and others.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * @section DESCRIPTION
 *
 * Definition of Libavoid's options and functions to setup the graph for
 * connector routing as well as to write the results back to an output stream.
 *
 * Protocol:
 *  - All nodes are passed together with a continuously increasing id starting by 1. (1 2 3 4 ...) 
 *  - The same goes for the edges. 
 */
#include <iostream>
#include <string>
#include <sstream>

#include "libavoid/libavoid.h"

/*
 * Debug
 */
//#define DEBUG_EXEC_TIME

/*
 * Edge Routing
 */
#define EDGE_ROUTING                "edgeRouting"
#define EDGE_ROUTING_POLYLINE       "POLYLINE"
#define EDGE_ROUTING_ORTHOGONAL     "ORTHOGONAL"

#define DIRECTION                   "direction"
#define DIRECTION_UNDEFINED         "UNDEFINED"
#define DIRECTION_UP                "UP"
#define DIRECTION_RIGHT             "RIGHT"
#define DIRECTION_DOWN              "DOWN"
#define DIRECTION_LEFT              "LEFT"

/*
 * Routing Penalties
 */
#define SEGMENT_PENALTY 			"segmentPenalty"
#define ANGLE_PENALTY 				"anglePenalty"
#define CROSSING_PENALTY 			"crossingPenalty"
#define CLUSTER_CROSSING_PENALTY 	"clusterCrossingPenalty"
#define FIXED_SHARED_PATH_PENALTY 	"fixedSharedPathPenalty"
#define PORT_DIRECTION_PENALTY		"portDirectionPenalty"
#define SHAPE_BUFFER_DISTANCE 		"shapeBufferDistance"
#define IDEAL_NUDGING_DISTANCE 		"idealNudgingDistance"
#define REVERSE_DIRECTION_PENALTY	"reverseDirectionPenalty"

/*
 * Routing Options
 */
#define NUDGE_ORTHOGONAL_SEGMENTS			"nudgeOrthogonalSegmentsConnectedToShapes"
#define IMPROVE_HYPEREDGES					"improveHyperedgeRoutesMovingJunctions"
#define PENALISE_ORTH_SHATE_PATHS			"penaliseOrthogonalSharedPathsAtConnEnds"
#define NUDGE_ORTHOGONAL_COLINEAR_SEGMENTS	"nudgeOrthogonalTouchingColinearSegments"
#define NUDGE_PREPROCESSING					"performUnifyingNudgingPreprocessingStep"
#define IMPROVE_HYPEREDGES_ADD_DELETE       "improveHyperedgeRoutesMovingAddingAndDeletingJunctions"
#define NUDGE_SHARED_PATHS_COMMON_ENDPOINT  "nudgeSharedPathsWithCommonEndPoint"

/*
 * Port Sides 
 */
#define PORT_SIDE_NORTH		"NORTH"
#define PORT_SIDE_EAST		"EAST"
#define PORT_SIDE_SOUTH		"SOUTH"
#define PORT_SIDE_WEST		"WEST"

/*
 * Pin Types
 *
 * Per definition the ids of passed ports start at 5. Thus, [1..4] are free for
 * arbitrary definition.
 */
/** Indicates pins that can be used by an arbitrary endpoint of an edge. */
const unsigned int PIN_ARBITRARY = 1;
/** Indicates pins reserved for incoming edges. */
const unsigned int PIN_INCOMING = 2;
/** Indicates pins reserved for outgoing edges. */
const unsigned int PIN_OUTGOING = 3;

/**
 * Assembling the graph
 */
void setPenalty(std::string optionId, std::string token, Avoid::Router* router);

void setOption(std::string optionId, std::string token, Avoid::Router* router);

void addNode(std::vector<std::string> &tokens, std::vector<Avoid::ShapeRef*> &shapes,
        Avoid::Router* router, std::string direction);

void addPort(std::vector<std::string> &tokens, std::vector<Avoid::ShapeConnectionPin*> &pins,
        std::vector<Avoid::ShapeRef*> &shapes, Avoid::Router* router);

void addEdge(std::vector<std::string> &tokens, Avoid::ConnType connectorType,
        std::vector<Avoid::ShapeRef*> &shapes, std::vector<Avoid::ConnRef*> &cons,
        Avoid::Router* router, std::string direction);

/**
 * Writing the graph to the output stream
 */
void writeLayout(std::ostream& out, std::vector<Avoid::ConnRef*> cons);

/*
 * Convenient methods
 */
inline double toDouble(std::string const& s) {
    std::istringstream i(s);
    double x;
    i >> x;

    return x;
}

inline int toInt(std::string const& s) {
    std::istringstream i(s);
    int x;
    i >> x;

    return x;
}

inline bool toBool(std::string const& s) {
    return (s == "true" || s == "TRUE" || s == "True");
}

void tokenize(std::string text, std::vector<std::string>& tokens);

