/**
 * @file    LibavoidRouting.cpp
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
 * The implementation of the functions defined in LibavoidRouting.h.
 */
#include "LibavoidRouting.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include "libavoid/libavoid.h"

using namespace std;

void tokenize(std::string text, std::vector<string>& tokens) {
    istringstream iss(text);
    copy(istream_iterator < string > (iss), istream_iterator<string>(),
            back_inserter < vector<string> > (tokens));
}

void setPenalty(string optionId, string token, Avoid::Router* router) {
    if (optionId.rfind("de.cau.cs.kieler.kiml.libavoid.", 0) == 0) {
        optionId = optionId.substr(31, std::string::npos);
    }
    float value = toDouble(token);

    if (optionId == SEGMENT_PENALTY) {
        router->setRoutingPenalty(Avoid::segmentPenalty, value);
    } else if (optionId == ANGLE_PENALTY) {
        router->setRoutingPenalty(Avoid::anglePenalty, value);
    } else if (optionId == CROSSING_PENALTY) {
        router->setRoutingPenalty(Avoid::crossingPenalty, value);
    } else if (optionId == CLUSTER_CROSSING_PENALTY) {
        router->setRoutingPenalty(Avoid::clusterCrossingPenalty, value);
    } else if (optionId == FIXED_SHARED_PATH_PENALTY) {
        router->setRoutingPenalty(Avoid::fixedSharedPathPenalty, value);
    } else if (optionId == PORT_DIRECTION_PENALTY) {
        router->setRoutingPenalty(Avoid::portDirectionPenalty, value);
    } else if (optionId == SHAPE_BUFFER_DISTANCE) {
        router->setRoutingPenalty(Avoid::shapeBufferDistance, value);
    } else if (optionId == IDEAL_NUDGING_DISTANCE) {
        router->setRoutingPenalty(Avoid::idealNudgingDistance, value);
    } else if (optionId == REVERSE_DIRECTION_PENALTY) {
        router->setRoutingPenalty(Avoid::reverseDirectionPenalty, value);
    } else {
        cerr << "ERROR: unknown penalty " << optionId << "." << endl;
    }
}

void setOption(string optionId, string token, Avoid::Router* router) {
    if (optionId.rfind("de.cau.cs.kieler.kiml.libavoid.", 0) == 0) {
        optionId = optionId.substr(31, std::string::npos);
    }
    bool value = toBool(token);

    if (optionId == NUDGE_ORTHOGONAL_SEGMENTS) {
        router->setRoutingOption(Avoid::nudgeOrthogonalSegmentsConnectedToShapes, value);
    } else if (optionId == IMPROVE_HYPEREDGES) {
        router->setRoutingOption(Avoid::improveHyperedgeRoutesMovingJunctions, value);
    } else if (optionId == PENALISE_ORTH_SHATE_PATHS) {
        router->setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds, value);
    } else if (optionId == NUDGE_ORTHOGONAL_COLINEAR_SEGMENTS) {
        router->setRoutingOption(Avoid::nudgeOrthogonalSegmentsConnectedToShapes, value);
    } else if (optionId == NUDGE_PREPROCESSING) {
        router->setRoutingOption(Avoid::performUnifyingNudgingPreprocessingStep, value);
    } else if (optionId == IMPROVE_HYPEREDGES_ADD_DELETE) {
        router->setRoutingOption(Avoid::improveHyperedgeRoutesMovingAddingAndDeletingJunctions, value);
    } else if (optionId == NUDGE_SHARED_PATHS_COMMON_ENDPOINT) {
        router->setRoutingOption(Avoid::nudgeSharedPathsWithCommonEndPoint, value);
    } else {
        cerr << "ERROR: unknown routing option " << optionId << "." << endl;
    }
}

void addNode(vector<string> &tokens, vector<Avoid::ShapeRef*> &shapes, Avoid::Router* router,
        string direction) {
    int id = toInt(tokens.at(1));
    double topLeftX = toDouble(tokens.at(2));
    double topLeftY = toDouble(tokens.at(3));
    double bottomRightX = toDouble(tokens.at(4));
    double bottomRightY = toDouble(tokens.at(5));
    int portLessIncomingEdges = toInt(tokens[6]);
    int portLessOutgoingEdges = toInt(tokens[7]);

    // add the actual rectangle
    Avoid::Rectangle rectangle(Avoid::Point(topLeftX, topLeftY),
            Avoid::Point(bottomRightX, bottomRightY));
    Avoid::ShapeRef *shapeRef = new Avoid::ShapeRef(router, rectangle, id);

    // remember in vector
    shapes.push_back(shapeRef);

    // create pins for port-less edges
    if (direction == DIRECTION_UNDEFINED) {

        // create incoming+outgoing pins on each side of the node
        int totalPins = portLessIncomingEdges + portLessOutgoingEdges;
        if (totalPins > 0) {
            double spacing = 1 / (double) (totalPins + 1);

            Avoid::ConnDirFlag connDir[] = { Avoid::ConnDirUp, Avoid::ConnDirRight, Avoid::ConnDirDown,
                    Avoid::ConnDirLeft };
            int xPos[] = { 1, 0, 1, 0 };
            int xOffset[] = { 0, 1, 0, 0 };
            int yPos[] = { 0, 1, 0, 1 };
            int yOffset[] = { 0, 0, 1, 0 };

            // create the pins on each side
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < totalPins; j++) {
                    Avoid::ShapeConnectionPin *pin = new Avoid::ShapeConnectionPin(shapeRef,
                            PIN_ARBITRARY, xPos[i] * ((j + 1) * spacing) + xOffset[i],
                            yPos[i] * ((j + 1) * spacing) + yOffset[i], 0.0, connDir[i]);
                    pin->setExclusive(true);

                    /*cout << "Pin at " << xPos[i] * ((j+1) * spacing) + xOffset[i] << " " <<
                     yPos[i] * ((j+1) * spacing) + yOffset[i] << " " <<
                     connDir[i] << endl;*/
                }
            }
        }
    } else {
        // create incoming pins on the "first" side, considering the direction,
        // outgoing edges on the "last" side

        // some setups depending on direction option
        Avoid::ConnDirFlag connDirIncoming;
        Avoid::ConnDirFlag connDirOutgoing;
        int horizontal = 0;
        int vertical = 0;
        // where are the incoming ports?
        int left = 0;
        int right = 0;
        int up = 0;
        int down = 0;

        if (direction == DIRECTION_RIGHT) {
            connDirIncoming = Avoid::ConnDirLeft;
            connDirOutgoing = Avoid::ConnDirRight;
            horizontal = 1;
            left = 1;
        } else if (direction == DIRECTION_LEFT) {
            connDirIncoming = Avoid::ConnDirRight;
            connDirOutgoing = Avoid::ConnDirLeft;
            horizontal = 1;
            right = 1;
        } else if (direction == DIRECTION_UP) {
            connDirIncoming = Avoid::ConnDirDown;
            connDirOutgoing = Avoid::ConnDirUp;
            vertical = 1;
            down = 1;
        } else if (direction == DIRECTION_DOWN) {
            connDirIncoming = Avoid::ConnDirUp;
            connDirOutgoing = Avoid::ConnDirDown;
            vertical = 1;
            up = 1;
        }

        // create the pins
        // incoming
        if (portLessIncomingEdges > 0) {
            double incSpacing = 1 / (double) (portLessIncomingEdges + 1);
            for (int i = 0; i < portLessIncomingEdges; i++) {
                Avoid::ShapeConnectionPin *pin = new Avoid::ShapeConnectionPin(shapeRef,
                        PIN_INCOMING, vertical * ((i + 1) * incSpacing) + right,
                        horizontal * ((i + 1) * incSpacing) + down, 0, connDirIncoming);
                pin->setExclusive(true);
            }
        }

        // outgoing
        if (portLessOutgoingEdges > 0) {
            double outSpacing = 1 / (double) (portLessOutgoingEdges + 1);
            for (int i = 0; i < portLessOutgoingEdges; i++) {
                Avoid::ShapeConnectionPin *pin = new Avoid::ShapeConnectionPin(shapeRef,
                        PIN_OUTGOING, vertical * ((i + 1) * outSpacing) + left,
                        horizontal * ((i + 1) * outSpacing) + up, 0, connDirOutgoing);
                pin->setExclusive(true);
            }
        }
    }
}

void addPort(vector<string> &tokens, vector<Avoid::ShapeConnectionPin*> &pins,
        vector<Avoid::ShapeRef*> &shapes, Avoid::Router* router) {
    // format: portId nodeId portSide centerX centerYs
    unsigned int portId = toInt(tokens[1]);
    unsigned int nodeId = toInt(tokens[2]);
    string side = tokens[3];

    // center positions of the ports
    double centerX = toDouble(tokens[4]);
    double centerY = toDouble(tokens[5]);

    Avoid::ShapeRef* shapeRef = shapes[nodeId - 1];
    Avoid::ShapeConnectionPin *pin;

    // get the bounding box of the node
    Avoid::Box box = shapeRef->polygon().offsetBoundingBox(0);
    // calculate width and height
    double width = box.max.x - box.min.x;
    double height = box.max.y - box.min.y;
    // determine the pin's positions relative on the respective side
    double relX = centerX / width;
    double relY = centerY / height;

    // create the pin with proper setup
    if (side == PORT_SIDE_NORTH) {
        pin = new Avoid::ShapeConnectionPin(shapeRef, portId, relX, Avoid::ATTACH_POS_TOP, 0,
                Avoid::ConnDirUp);
    } else if (side == PORT_SIDE_EAST) {
        pin = new Avoid::ShapeConnectionPin(shapeRef, portId, Avoid::ATTACH_POS_RIGHT, relY, 0,
                Avoid::ConnDirRight);
    } else if (side == PORT_SIDE_SOUTH) {
        pin = new Avoid::ShapeConnectionPin(shapeRef, portId, relX, Avoid::ATTACH_POS_BOTTOM, 0,
                Avoid::ConnDirDown);
    } else { // (side == PORT_SIDE_WEST) {
        pin = new Avoid::ShapeConnectionPin(shapeRef, portId, Avoid::ATTACH_POS_LEFT, relY, 0,
                Avoid::ConnDirLeft);
    }

    pin->setExclusive(false);
    pins.push_back(pin);
}

void addEdge(vector<string> &tokens, Avoid::ConnType connectorType, vector<Avoid::ShapeRef*> &shapes,
        vector<Avoid::ConnRef*> &cons, Avoid::Router* router, string direction) {
    int edgeId = toInt(tokens.at(1));
    int srcId = toInt(tokens.at(2));
    int tgtId = toInt(tokens.at(3));

    // get the shapes for the src and tgt node
    Avoid::ShapeRef *srcShape = shapes[srcId - 1];
    Avoid::ShapeRef *tgtShape = shapes[tgtId - 1];

    // determine the pin locations for this edge
    unsigned int srcPin = PIN_ARBITRARY;
    unsigned int tgtPin = PIN_ARBITRARY;

    // differenciate the edge types
    if (tokens[0] == "PEDGEP") {
        srcPin = toInt(tokens[4]);
        tgtPin = toInt(tokens[5]);
    } else if (tokens[0] == "PEDGE") {
        srcPin = toInt(tokens[4]);
        // set port-less pin
        if (direction != DIRECTION_UNDEFINED) {
            tgtPin = PIN_INCOMING;
        }
    } else if (tokens[0] == "EDGEP") {
        // set port-less pin
        if (direction != DIRECTION_UNDEFINED) {
            srcPin = PIN_OUTGOING;
        }
        tgtPin = toInt(tokens[5]);
    } else {
        // no port on each side
        if (direction != DIRECTION_UNDEFINED) {
            tgtPin = PIN_INCOMING;
            srcPin = PIN_OUTGOING;
        }
    }

    // create endpoints
    Avoid::ConnEnd srcPt(srcShape, srcPin);
    Avoid::ConnEnd tgtPt(tgtShape, tgtPin);

    // create the connector
    Avoid::ConnRef *connRef = new Avoid::ConnRef(router, srcPt, tgtPt, edgeId);
    connRef->setRoutingType(connectorType);

    cons.push_back(connRef);
}

void writeLayout(ostream& out, vector<Avoid::ConnRef*> cons) {
    out << "LAYOUT" << endl;

    for (std::vector<int>::size_type i = 0; i != cons.size(); i++) {

        out << "EDGE " << cons[i]->id() << "=";

		// Be sure to use #displayRoute() here and not route(), as the 
		// second method only contains the "raw" route, eg, without any
		// nudging done.
        const Avoid::PolyLine route = cons[i]->displayRoute();
        for (size_t i = 0; i < route.ps.size(); ++i) {
            out << route.ps[i].x << " " << route.ps[i].y << " ";
        }
        out << endl;
    }

    out << "DONE" << endl;
}
