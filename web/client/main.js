(function(){

var url = "ws://localhost:9004/";
window.onload = function main() {
    var ws;

    // FireFoxとの互換性を考慮してインスタンス化
    if ("WebSocket" in window) {
        ws = new WebSocket(url);
    } else if ("MozWebSocket" in window) {
        ws = new MozWebSocket(url);
    }

    // メッセージ受信時のコールバック関数
    ws.onmessage = function(event) {
        console.log("受信メッセージ:" + event.data);
    }
    //entry point
    ws.onopen = function(){
        console.log("on open");
        ws.send("123");
        ws.send("456");
    };
}

})()