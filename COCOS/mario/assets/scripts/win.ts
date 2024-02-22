import { _decorator, Component, director, Node } from 'cc';
const { ccclass, property } = _decorator;

@ccclass('win')
export class win extends Component {
    start() {
        this.scheduleOnce(function () {
            director.loadScene("scene2");
        }, 5);
    }
}


