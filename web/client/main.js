var cl;
$(function() {
    var url = "ws://localhost:9002/";
    window.onload = function main() {
        function Client() {
            var self={};
            var ws;
            // FireFoxとの互換性を考慮してインスタンス化
            if ("WebSocket" in window) {
                ws = new WebSocket(url);
            } else if ("MozWebSocket" in window) {
                ws = new MozWebSocket(url);
            }
            self.start = function start() {
                ws.onopen = function() {
                    console.log("Connection Established.");
                    ws.send("TELL RANGE");
                    ws.onmessage = function(event) {
                        var range = event.data.split(":");
                        self.min = parseInt(range[0], 10);
                        self.max = parseInt(range[1], 10);
                        console.log("RANGE: " + self.min + " -> " + self.max);
                        console.log( $( "#selector" ).slider());
                        ws.onmessage = null;
                    }
                };
            }

            self.receive = function receive(at) {
                ws.send(""+Math.round(at)+":"+cl.max);
                ws.onmessage = function(event) {
                    console.log("received");
                    var data = JSON.parse(event.data);
                    visualize(data);
                };
            }
            return self;
        };
        cl = Client();
        cl.start();
    }
    var nodes = new vis.DataSet();
    var edges = new vis.DataSet();
    var datas = {nodes: nodes, edges: edges};
    var network;
    function visualize(payload){
      // create a network
      nodes.clear();
      edges.clear();
      nodes.add(payload.nodes);
      edges.add(payload.edges);
      var container = document.getElementById('visualize');
      var options = {
        width:  '1200px',
        height: '1200px',
        nodes: {
            shape: 'dot',
            radiusMin: 10,
            radiusMax: 30,
            fontSize: 12,
            fontFace: "Tahoma",
            fontFill: "white"
            },
        edges: {
            width: 0.15,
            inheritColor: "from"
            },
        tooltip: {
            delay: 200,
            fontSize: 12,
            color: {
                background: "#fff"
                }
            },
          stabilize: false,
        smoothCurves: {dynamic:false, type: "continuous"},
        physics: {barnesHut: {gravitationalConstant: -80000, springConstant: 0.001, springLength: 200}},
        hideEdgesOnDrag: true
      };
      network = new vis.Network(container, datas, options);
      network.on("click",onClick);
  }

function onClick(selectedItems) {
    var nodeId;
    var degrees = 2;
    // we get all data from the dataset once to avoid updating multiple times.
    var allNodes = nodes.get({returnType:"Object"});
    if (selectedItems.nodes.length == 0) {
        // restore on unselect
        for (nodeId in allNodes) {
            if (allNodes.hasOwnProperty(nodeId)) {
                allNodes[nodeId].color = undefined;
                if (allNodes[nodeId].oldLabel !== undefined) {
                    allNodes[nodeId].label = allNodes[nodeId].oldLabel;
                    allNodes[nodeId].oldLabel = undefined;
                }
                allNodes[nodeId]['levelOfSeperation'] = undefined;
                allNodes[nodeId]['inConnectionList'] = undefined;
            }
        }
    }
    else {
        // we clear the level of separation in all nodes.
        clearLevelOfSeperation(allNodes);

        // we will now start to collect all the connected nodes we want to highlight.
        var connectedNodes = selectedItems.nodes;

        // we can store them into levels of separation and we could then later use this to define a color per level
        // any data can be added to a node, this is just stored in the nodeObject.
        storeLevelOfSeperation(connectedNodes,0, allNodes);
        for (var i = 1; i < degrees + 1; i++) {
            appendConnectedNodes(connectedNodes);
            storeLevelOfSeperation(connectedNodes, i, allNodes);
        }
        for (nodeId in allNodes) {
            if (allNodes.hasOwnProperty(nodeId)) {
                if (allNodes[nodeId]['inConnectionList'] == true) {
                    if (allNodes[nodeId]['levelOfSeperation'] !== undefined) {
                        if (allNodes[nodeId]['levelOfSeperation'] >= 2) {
                            allNodes[nodeId].color = 'rgba(150,150,150,0.75)';
                        }
                        else {
                            allNodes[nodeId].color = undefined;
                        }
                    }
                    else {
                        allNodes[nodeId].color = undefined;
                    }
                    if (allNodes[nodeId].oldLabel !== undefined) {
                        allNodes[nodeId].label = allNodes[nodeId].oldLabel;
                        allNodes[nodeId].oldLabel = undefined;
                    }
                }
                else {
                    allNodes[nodeId].color = 'rgba(200,200,200,0.5)';
                    if (allNodes[nodeId].oldLabel === undefined) {
                        allNodes[nodeId].oldLabel = allNodes[nodeId].label;
                        allNodes[nodeId].label = "";
                    }
                }
            }
        }
    }
    var updateArray = [];
    for (nodeId in allNodes) {
        if (allNodes.hasOwnProperty(nodeId)) {
            updateArray.push(allNodes[nodeId]);
        }
    }
    nodes.update(updateArray);
}
/**
 * update the allNodes object with the level of separation.
 * Arrays are passed by reference, we do not need to return them because we are working in the same object.
 */
function storeLevelOfSeperation(connectedNodes, level, allNodes) {
    for (var i = 0; i < connectedNodes.length; i++) {
        var nodeId = connectedNodes[i];
        if (allNodes[nodeId]['levelOfSeperation'] === undefined) {
            allNodes[nodeId]['levelOfSeperation'] = level;
        }
        allNodes[nodeId]['inConnectionList'] = true;
    }
}

function clearLevelOfSeperation(allNodes) {
    for (var nodeId in allNodes) {
        if (allNodes.hasOwnProperty(nodeId)) {
            allNodes[nodeId]['levelOfSeperation'] = undefined;
            allNodes[nodeId]['inConnectionList'] = undefined;
        }
    }
}

/**
 * Add the connected nodes to the list of nodes we already have
 *
 *
 */
function appendConnectedNodes(sourceNodes) {
    var tempSourceNodes = [];
    // first we make a copy of the nodes so we do not extend the array we loop over.
    for (var i = 0; i < sourceNodes.length; i++) {
        tempSourceNodes.push(sourceNodes[i])
    }

    for (var i = 0; i < tempSourceNodes.length; i++) {
        var nodeId = tempSourceNodes[i];
        if (sourceNodes.indexOf(nodeId) == -1) {
            sourceNodes.push(nodeId);
        }
        var connectedNodes = network.getConnectedNodes(nodeId);
        addUnique(connectedNodes,sourceNodes);
    }
    tempSourceNodes = null;
}

/**
 * Join two arrays without duplicates
 * @param fromArray
 * @param toArray
 */
function addUnique(fromArray, toArray) {
    for (var i = 0; i < fromArray.length; i++) {
        if (toArray.indexOf(fromArray[i]) == -1) {
            toArray.push(fromArray[i]);
        }
    }
}
});
$(function() {
    $("#slider").slider({
        change: function(event, ui) {
            console.log(ui.value);
            cl.receive(((cl.max-cl.min) * ui.value /1000)+cl.min);
        },
        slide: function(event, ui) {
            console.log(ui.value);
        },
        min: 0,
        max: 1000
    });
});