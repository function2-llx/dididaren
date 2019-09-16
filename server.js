const express = require('express')
const path = require('path')
const app = express()
var bodyParser = require('body-parser')

app.use(express.static(path.join(__dirname, 'site')))
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({//处理以form表单的提交
    extended: true
}))

app.listen(8080, () => {
  console.log('App listening at port 8080')
})


app.post('/select', function(req, res) {
    var x = Number(req.body['x']), y = Number(req.body['y'])
    database.getPos(x, y, function(x, y) {
        res.json({
            x: x,
            y: y
        })
    })
})

app.post('/get-cars', function(req, res) {
    console.log('get cars lo')
    // console.log(req.body)
    let origin = req.body.origin, dest = req.body.dest
    database.getCars(origin.x, origin.y, dest.x, dest.y, function(carInfos) {
        res.json({
            status: 0,
            info: carInfos
        })
    })
})


const database = require("./build/Release/database")
console.log(database.testInterface())