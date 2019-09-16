let filename = 'data/road.cnode'

// let fso = new ActiveXObject("Scripting.FileSystemObject"); 
// let ts = fso.OpenTextFile(filename, ForReading);
// let n = 338024
// for (let i = 0; i < n; i++) {
//     ts.readLine()
//     console.log(ts)
// }

const n = 338024
let cnt = 0

const fs = require('fs')
fs.readFile(filename, function(err, data) {
    if (err)
        throw err

    data = data.toString().split('\n').map(v => v.split(' ').map(v => Number(v)))
    console.log(data.length)
    // console.log(data)
    console.log('read over')
    app.listen(8080, () => {
        console.log('App listening at port 8080')
    })

    let fd = fs.openSync('data/bnode.txt', 'a')

    app.post('/trans', function(req, res) {
        // console.log(2333)
        let l = Number(req.body.l), r = Number(req.body.r)
        console.log('trans', l, r)
        // console.log(id)
        let points = []
        for (let i = l; i < r; i++) {
            console.log(i)
            points.push({
                x: data[i][1],
                y: data[i][2]
            })
        }

        res.json({
            status: 0,
            points: points
        })
    })

    app.post('/get-result', function(req, res) {
        let result = req.body.result
        cnt += result.length
        for (let point of result) {
            let id = point.id, x = point.x, y = point.y
            console.log(point)
            data[id].push(x, y)
            fs.writeSync(fd, point.id + ' ' + point.x + ' ' + point.y + '\n')
        }
        res.end()
    })
})

const express = require('express')
const path = require('path')
const app = express()
var bodyParser = require('body-parser')

app.use(express.static(path.join(__dirname, 'site')))
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({//处理以form表单的提交
    extended: true
}))
