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
            inheritColor: "from",
            color:{
                color: 'rgba(150,150,150,0.3)',
                highlight: 'rgba(43,124,233,0.5)'
            }
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
        hideEdgesOnDrag: true,
        smoothCurves: false
      };
      network = new vis.Network(container, datas, options);
      network.on("click",onClick);
      network.on("doubleClick",onDoubleClick);
  }

var nonselected = 'rgba(150,150,150,0.3)';
var selected = {
    background: '#97C2FC',
    border: '#2B7CE9',
};
function onClick(selectedItems) {
    var fromId = selectedItems.nodes[0];
    var connected = network.getConnectedNodes(fromId);
    var allNodes = nodes.get({returnType:"Object"});
    var changed = [];
    for(nodeId in allNodes){
        if(allNodes.hasOwnProperty(nodeId)){
            var index = connected.indexOf(parseInt(nodeId));
            if(nodeId !== fromId && index < 0){
                if(allNodes[nodeId].color !== nonselected){
                    allNodes[nodeId].color = nonselected;
                    changed.push(allNodes[nodeId]);
                }
            }else{
                if(allNodes[nodeId].color !== selected){
                    allNodes[nodeId].color = selected;
                    changed.push(allNodes[nodeId]);
                }
            }
        }
    }
    nodes.update(changed);
}
function onDoubleClick(selectedItems) {
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