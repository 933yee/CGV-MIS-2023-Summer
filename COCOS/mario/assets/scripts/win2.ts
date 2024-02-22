import { _decorator, Component, director, Node } from 'cc';
const { ccclass, property } = _decorator;

@ccclass('win2')
export class win2 extends Component {
    start() {
        this.scheduleOnce(function () {
            director.loadScene("menu");
        }, 5);
    }
}


