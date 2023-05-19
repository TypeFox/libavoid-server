/**
 * @file    main.cpp
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
 * Main entry point for the libavoid-server application.
 */
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include "ChunkStream.h"
#include "libavoid/libavoid.h"
#include "LibavoidRouting.h"

using namespace std;

/* The keyword used to separate parts of the data transmission. */
#define CHUNK_KEYWORD "[CHUNK]\n"

/**
 * Handles a layout request, which consists of reading the graph and layout
 * options from the input stream, performing the actual connector routing
 * using the Libavoid library and writing the results back to an output stream.
 *
 * @param in
 *            the input stream
 * @param out
 *            the output stream
 */
void HandleRequest(chunk_istream& stream, ostream& out);

/**
 * The program entry point.
 */
int main(void) {

    // handle requests from stdin, writes to stdout
    chunk_istream chunkStream(cin, CHUNK_KEYWORD);
    while (!chunkStream.isRealEof()) {
        HandleRequest(chunkStream, cout);
        chunkStream.nextChunk();
    }

    return 0;
}

void HandleRequest(chunk_istream& stream, ostream& out) {

    vector<Avoid::ShapeRef *> shapes;
    vector<Avoid::ShapeConnectionPin *> pins;
    vector<Avoid::ConnRef *> cons;

	// router is initialized upon receiption of the edge routing option
    Avoid::Router *router = 0;

    // options
    Avoid::ConnType connectorType = Avoid::ConnType_Orthogonal;
    std::string direction = DIRECTION_UNDEFINED;

	// should we print debug information?
	bool debug = false;
    // has the graph declaration started?
    bool graphDecl = false;
    // have hyperedges been enabled? will result in decreased performance
    bool hyperedges = false;

    // read graph from stdin
    for (std::string line; std::getline(std::cin, line);) {

        // split the line into its parts
        vector<string> tokens;
        tokenize(line, tokens);

        if (tokens.size() >= 3 && tokens[0] == "PENALTY") {
            if (router == NULL) {
                router = new Avoid::Router(Avoid::OrthogonalRouting);
            }
            if (graphDecl) {
                cerr << "WARNING: penalties should not be specified after GRAPH declaration" << endl;
            }

            /* Penalties */
            setPenalty(tokens[1], tokens[2], router);

        } else if (tokens.size() >= 3 && tokens[0] == "ROUTINGOPTION") {
            if (router == NULL) {
                router = new Avoid::Router(Avoid::OrthogonalRouting);
            }
            if (graphDecl) {
                cerr << "WARNING: routing options should not be specified after GRAPH declaration" << endl;
            }

            /* Routing options */
            setOption(tokens[1], tokens[2], router);

        } else if (tokens.size() >= 3 && tokens[0] == "OPTION") {
            if (graphDecl) {
                cerr << "WARNING: options should not be specified after GRAPH declaration" << endl;
            }
            std::string optionId = tokens[1];
            if (optionId.rfind("org.eclipse.elk.", 0) == 0) {
                optionId = optionId.substr(16, std::string::npos);
            } else if (optionId.rfind("de.cau.cs.kieler.", 0) == 0) {
                optionId = optionId.substr(17, std::string::npos);
            }

            /* General options */
            if (optionId == EDGE_ROUTING) {
				if (router) {
					// possibly delete an old router
                    cerr << "WARNING: discarding previous options due to " << EDGE_ROUTING << " declaration." << endl;
					delete router;
				}
                // edge routing
                if (tokens[2] == EDGE_ROUTING_POLYLINE) {
					router = new Avoid::Router(Avoid::PolyLineRouting);
                    connectorType = Avoid::ConnType_PolyLine;
                } else {
                    // default orthogonal
					router = new Avoid::Router(Avoid::OrthogonalRouting);
                    connectorType = Avoid::ConnType_Orthogonal;
                }
            } else if (optionId == DIRECTION) {
                // layout direction
                direction = tokens[2];
            } else if (optionId == ENABLE_HYPEREDGES_FROM_COMMON_SOURCE) {
                hyperedges = true;
            } else {
                cerr << "ERROR: unknown option " << tokens[1] << "." << endl;
            }

        } else if (tokens[0] == "NODE") {
            if (router == NULL) {
                router = new Avoid::Router(Avoid::OrthogonalRouting);
            }
            if (!graphDecl) {
                cerr << "ERROR: missing declaration of GRAPH" << endl;
                graphDecl = true;
            }
            // format:
            // id topleft bottomright portLessIncomingEdges portLessOutgoingEdges
            if (tokens.size() != 8) {
                cerr << "ERROR: invalid node format" << endl;
            }

            addNode(tokens, shapes, router, direction);

        } else if (tokens[0] == "CLUSTER") {
            if (router == NULL) {
                router = new Avoid::Router(Avoid::OrthogonalRouting);
            }
            if (!graphDecl) {
                cerr << "ERROR: missing declaration of GRAPH" << endl;
                graphDecl = true;
            }
            // format:
            // id topleft bottomright
            if (tokens.size() != 6) {
                cerr << "ERROR: invalid cluster format" << endl;
            }

            addCluster(tokens, router);

        } else if (tokens[0] == "PORT") {
            if (router == NULL) {
                router = new Avoid::Router(Avoid::OrthogonalRouting);
            }
            if (!graphDecl) {
                cerr << "ERROR: missing declaration of GRAPH" << endl;
                graphDecl = true;
            }
            // format: portId nodeId portSide centerX centerYs
            if (tokens.size() != 6) {
                cerr << "ERROR: invalid port format" << endl;
            }

            addPort(tokens, pins, shapes, router);

        } else if (tokens[0] == "EDGE" || tokens[0] == "PEDGEP" || tokens[0] == "PEDGE"
                || tokens[0] == "EDGEP") {
            if (router == NULL) {
                router = new Avoid::Router(Avoid::OrthogonalRouting);
            }
            if (!graphDecl) {
                cerr << "ERROR: missing declaration of GRAPH" << endl;
                graphDecl = true;
            }
            // format: edgeId srcId tgtId srcPort tgtPort
            if (tokens.size() != 6) {
                cerr << "ERROR: invalid edge format" << endl;
            }

            addEdge(tokens, connectorType, shapes, cons, router, direction);

		} else if (tokens[0] == "DEBUG") {
			debug = true;

        } else if (tokens[0] == "GRAPH") {
            if (graphDecl) {
                cerr << "ERROR: duplicate declaration of GRAPH" << endl;
            }
            graphDecl = true;
        } else if (tokens[0] == "GRAPHEND") {
            if (!graphDecl) {
                cerr << "ERROR: missing declaration of GRAPH" << endl;
            }
            break;
        } else if (tokens[0] == "#") {
            // ignore it
        } else {
            cerr << "ERROR: invalid command " << tokens[0] << "." << endl;
        }
    }
    if (router == NULL) {
        return;
    }

#ifdef DEBUG_EXEC_TIME
    // measure execution time of the routing process
    LARGE_INTEGER frequency;
    LARGE_INTEGER t1, t2;
    QueryPerformanceFrequency(&frequency); // ticks per second
    QueryPerformanceCounter(&t1); // first timestamp
#endif

    // perform edge routing
    router->processTransaction();
    if (hyperedges) {
        createHyperedges(cons, router);
    }

#ifdef DEBUG_EXEC_TIME
    QueryPerformanceCounter(&t2);
    // compute and print the elapsed time in millisec
    double elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
    cout << "DEBUG Execution time edge routing: " << elapsedTime << "ms." << endl;
#endif

	if (debug) {
		router->outputInstanceToSVG();
	}

    // write the layout to std out
    writeLayout(cout, cons);

    // cleanup
    delete router;

}

