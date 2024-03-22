const C_RED = '#dc322f';
const C_ORANGE = '#b58900';
const C_YELLOW = '#859900';
const C_GREEN = '#268bd2';
const C_BLUE = '#6c71c4';
const C_PURPLE = '#d33682';
const COLORS = [C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_BLUE, C_PURPLE];

let charts = [];
function buildChart(name, inputs){
    let canvas = document.getElementById(name);
    let d = inputs.map((l, i) => {
        return {
            label: l,
            backgroundColor: COLORS[i % 6],
            borderColor: COLORS[i % 6],
            data:[]
        };
    });
    let config = {
            type: 'line',
            data: {
                labels: [],
                datasets: d,
            },
            options: {
                animation: false,
                scales: {
                    x: {
                        ticks: {
                            color: "#839496",
                        },
                        grid: {
                            color: "#839496",
                        },
                    },
                    y: {
                        ticks: {
                            color: "#839496",
                        },
                        grid: {
                            color: "#839496",
                        },
                    },
                },
                color: '#839496',
            },
        };
    let chart = new Chart(canvas, config);
    charts.push(chart);
}
buildChart('speChartL', ['mtl', 'msl']);
buildChart('speChartR', ['mtr', 'msr']);
buildChart('encChart', ['mel', 'mer']);
buildChart('bavChart', ['bav']);

