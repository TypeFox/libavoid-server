# Libavoid Server

This project wraps [libavoid](https://www.adaptagrams.org/documentation/libavoid.html) into an executable that can run in the background of an application to provide edge routing for graphs with fixed node positions.

## Configuration

The program listens for requests to standard input and writes results to standard output. Edge routing requests are separated with the character sequence
```
[CHUNK]
```
followed by a new line character (`\n`). All other commands documented below must be written in separate lines as well.

### General Options

A general [layout option](https://www.eclipse.org/elk/reference/options.html) is applied using a line with the format
```
OPTION {id} {value}
```
where `{id}` is the identifier of the layout option and `{value}` is a value for that option. The following layout options are supported:

 * [`edgeRouting`](https://www.eclipse.org/elk/reference/options/org-eclipse-elk-edgeRouting.html)
 * [`direction`](https://www.eclipse.org/elk/reference/options/org-eclipse-elk-direction.html)

_Note:_ The `edgeRouting` option must be applied before all other configuration parameters.

### Routing Options

A [routing option](https://www.adaptagrams.org/documentation/classAvoid_1_1Router.html#a09f057f6d101f010588c9022893c9ac1) is applied using a line with the format
```
ROUTINGOPTION {id} {value}
```
where `{id}` is the identifier of the routing option and `{value}` is a Boolean value to assign. The following routing option ids are available:

 * `nudgeOrthogonalSegmentsConnectedToShapes`
 * `improveHyperedgeRoutesMovingJunctions`
 * `penaliseOrthogonalSharedPathsAtConnEnds`
 * `nudgeOrthogonalTouchingColinearSegments`
 * `performUnifyingNudgingPreprocessingStep`
 * `improveHyperedgeRoutesMovingAddingAndDeletingJunctions`

Their meaning is documented in the [libavoid documentation](https://www.adaptagrams.org/documentation/namespaceAvoid.html#abc707ccbd6a0a7c29c124162c864ca05).

### Routing Penalties

A [routing penalty](https://www.adaptagrams.org/documentation/classAvoid_1_1Router.html#acbda0590ff3234faad409e2f39e1c9ec) is applied using a line with the format
```
PENALTY {id} {value}
```
where `{id}` is the identifier of the routing penalty and `{value}` is a numeric value to assign. The following penalty ids are available:

 * `segmentPenalty`
 * `anglePenalty`
 * `crossingPenalty`
 * `clusterCrossingPenalty`
 * `fixedSharedPathPenalty`
 * `portDirectionPenalty`
 * `shapeBufferDistance`
 * `idealNudgingDistance`

Their meaning is documented in the [libavoid documentation](https://www.adaptagrams.org/documentation/namespaceAvoid.html#a8a0154ae39129e7737d98e5a83daed19).

## Defining the Input Graph

Once all parameters have been set, you start the graph definition with the line
```
GRAPH
```
and you end it with the line
```
GRAPHEND
```

### Nodes

Between the two lines delimiting the graph, a node is added using a line with the format
```
NODE {id} {x1} {y1} {x2} {y2} {incoming} {outgoing}
```
with the following placeholders:

 * `{id}` &ndash; numeric (integer) identifier of the node
 * `{x1}` &ndash; horizontal position of the top left corner
 * `{y1}` &ndash; vertical position of the top left corner
 * `{x2}` &ndash; horizontal position of the bottom right corner
 * `{y2}` &ndash; vertical position of the bottom right corner
 * `{incoming}` &ndash; number of incoming edges that are not connected to a port
 * `{outgoing}` &ndash; number of outgoing edges that are not connected to a port

### Ports

A port is added to a node using a line with the format
```
PORT {port id} {node id} {side} {x} {y}
```
with the following placeholders:

 * `{port id}` &ndash; numeric (integer) identifier of the port
 * `{node id}` &ndash; identifier of the node to which the port belongs
 * `{side}` &ndash; side of the node on which the port is placed, either `NORTH`, `EAST`, `SOUTH` or `WEST`
 * `{x}` &ndash; horizontal position of the port center
 * `{y}` &ndash; vertical position of the port center

### Edges

An edge is added between two nodes or ports using a line with the format
```
{edge type} {edge id} {source node id} {target node id} {source port id} {target port id}
```
with the following placeholders:

 * `{edge type}` &ndash; `PEDGEP` for a port-to-port edge, `PEDGE` for a port-to-node edge, `EDGEP` for a node-to-port edge, or `EDGE` for a node-to-node edge
 * `{edge id}` &ndash; numeric (integer) identifier of the edge
 * `{source node id}` &ndash; identifier of the source node (or the node containing the source port)
 * `{target node id}` &ndash; identifier of the target node (or the node containing the target port)
 * `{source port id}` &ndash; identifier of the source port (ignored if there is none)
 * `{target port id}` &ndash; identifier of the target port (ignored if there is none)

## Output Format

The output is written to stdout. It starts with the line

```
LAYOUT
```

and ends with the line

```
DONE
```

### Edge Layouts

The actual content of the output is written between the two delimiter lines mentioned above. An edge layout has the format
```
EDGE {id}={route}
```
with the following placeholders:

 * `{id}` &ndash; identifier of the edge
 * `{route}` &ndash; space-separated list of points specifying the route; each point is a pair of x/y positions

## Example

Input:
```
OPTION edgeRouting ORTHOGONAL
PENALTY segmentPenalty 10.0
PENALTY anglePenalty 0.0
PENALTY crossingPenalty 0.0
PENALTY clusterCrossingPenalty 4000.0
PENALTY fixedSharedPathPenalty 0.0
PENALTY portDirectionPenalty 100.0
PENALTY shapeBufferDistance 4.0
PENALTY idealNudgingDistance 0.0
PENALTY REVERSE_DIRECTION_PENALTY 0.0
ROUTINGOPTION nudgeOrthogonalSegmentsConnectedToShapes false
ROUTINGOPTION improveHyperedgeRoutesMovingJunctions true
ROUTINGOPTION penaliseOrthogonalSharedPathsAtConnEnds false
ROUTINGOPTION nudgeOrthogonalTouchingColinearSegments false
ROUTINGOPTION performUnifyingNudgingPreprocessingStep true
ROUTINGOPTION improveHyperedgeRoutesMovingAddingAndDeletingJunctions true
ROUTINGOPTION nudgeSharedPathsWithCommonEndPoint true
GRAPH
NODE 1 0.0 0.0 0.0 0.0 0 0
NODE 2 0.0 0.0 0.0 0.0 0 0
NODE 3 0.0 0.0 0.0 0.0 0 0
NODE 4 0.0 0.0 0.0 0.0 0 0
NODE 5 20.0 20.0 50.0 50.0 0 0
PORT 5 5 EAST 32.5 12.5
PORT 6 5 EAST 32.5 22.5
NODE 6 80.0 20.0 110.0 50.0 0 0
PORT 7 6 WEST -2.5 14.5
NODE 7 150.0 30.0 180.0 60.0 0 0
PORT 8 7 WEST -2.5 14.5
PEDGEP 1 5 6 5 7
PEDGEP 2 5 7 6 8
GRAPHEND
```

Output:
```
LAYOUT
EDGE 1=50 32.5 65 32.5 65 34.5 80 34.5 
EDGE 2=50 42.5 65 42.5 65 54 130 54 130 44.5 150 44.5 
DONE
```

## License

This project is licensed under [Eclipse Public License v2.0](https://www.eclipse.org/legal/epl-2.0/). The libavoid library is licensed under [GNU Lesser General Public License v2.1](https://github.com/mjwybrow/adaptagrams/blob/master/cola/LICENSE) and its source code is available at [mjwybrow/adaptagrams](https://github.com/mjwybrow/adaptagrams).
