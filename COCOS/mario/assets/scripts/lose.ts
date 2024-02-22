import { _decorator, Component, director, Node } from 'cc';
const { ccclass, property } = _decorator;

@ccclass('lose')
export class lose extends Component {
    start() {
        this.scheduleOnce(function () {
            director.loadScene("menu");
        }, 5);
    }
}


