const mapCanvas = document.getElementById("map");
const mapSize = mapCanvas.getBoundingClientRect().height;
const mapCtx = mapCanvas.getContext("2d");
function mapInit() {
    console.log(mapCanvas);
    drawMap();
}

function drawMap() {
    mapCtx.lineWidth = 1;
    mapCtx.strokeStyle = "#839496";
    for(var i = 0; i < 5; i ++){
        mapCtx.beginPath();
        mapCtx.moveTo(100 + i * 100, 0);
        mapCtx.lineTo(100 + i * 100, 600);
        mapCtx.stroke();
        mapCtx.beginPath();
        mapCtx.moveTo(0, 100 + i * 100);
        mapCtx.lineTo(600, 100 + i * 100);
        mapCtx.stroke();
    }
}
