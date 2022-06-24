/**
 * @file    main.cpp
 * @author  uru
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * KIELER - Kiel Integrated Environment for Layout Eclipse RichClient
 *
 * http://www.informatik.uni-kiel.de/rtsys/kieler/
 *
 * Copyright 2013 by
 * + Kiel University
 *   + Department of Computer Science
 *     + Real-Time and Embedded Systems Group
 *
 * This code is provided under the terms of the Eclipse Public License (EPL).
 * See the file epl-v10.html for the license text.
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

    cout << "Terminating ..." << endl;

    return 0;
}

void HandleRequest(chunk_istream& stream, ostream& out) {

    vector<Avoid::ShapeRef *> shapes;
    vector<Avoid::ShapeConnectionPin *> pins;
    vector<Avoid::ConnRef *> cons;

	// router is initialized upon receiption of the edge routing option
    Avoid::Router *router = 0;

    // options
    Avoid::ConnType connectorType = Avoid::ConnType_PolyLine;
    std::string direction = DIRECTION_UNDEFINED;

	// should we print debug information?
	bool debug = false;

    // read graph from stdin
    for (std::string line; std::getline(std::cin, line);) {

        // split the line into its parts
        vector < string > tokens;
        tokenize(line, tokens);

        if (tokens[0] == "PENALTY") {

            /* Penalties */
            setPenalty(tokens[1], tokens[2], router);

        } else if (tokens[0] == "ROUTINGOPTION") {

            /* Routing options */
            setOption(tokens[1], tokens[2], router);

        } else if (tokens[0] == "OPTION") {

            /* General options */
            if (tokens[1] == EDGE_ROUTING || tokens[1] == EDGE_ROUTING_ELK) {
				if (router) {
					// possibly delete an old router
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
            } else if (tokens[1] == DIRECTION || tokens[1] == DIRECTION_ELK) {
                // layout direction
                direction = tokens[2];
            }

        } else if (tokens.at(0) == "NODE") {
            // format:
            // id topleft bottomright portLessIncomingEdges portLessOutgoingEdges
            if (tokens.size() != 8) {
                cerr << "ERROR: invalid node format" << endl;
            }

            addNode(tokens, shapes, router, direction);

        } else if (tokens[0] == "PORT") {
            // format: portId nodeId portSide centerX centerYs
            if (tokens.size() != 6) {
                cerr << "ERROR: invalid port format" << endl;
            }

            addPort(tokens, pins, shapes, router);

        } else if (tokens[0] == "EDGE" || tokens[0] == "PEDGEP" || tokens[0] == "PEDGE"
                || tokens[0] == "EDGEP") {
            // format: edgeId srcId tgtId srcPort tgtPort
            if (tokens.size() != 6) {
                cerr << "ERROR: invalid edge format" << endl;
            }

            addEdge(tokens, connectorType, shapes, cons, router, direction);

		} else if (tokens[0] == "DEBUG") {
			debug = true;

        } else if (tokens.at(0) == "GRAPHEND") {
            break;
        } else {
            // ignore it
        }

        //std::cout << line << std::endl;
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

