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
                };
            }
            return self;
        };
        cl = Client();
        cl.start();
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