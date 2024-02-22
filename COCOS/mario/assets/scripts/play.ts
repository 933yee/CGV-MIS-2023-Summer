import { _decorator, Component, director, Node } from 'cc';
const { ccclass, property } = _decorator;

@ccclass('play')
export class play extends Component {
    @property(Node)
    title: Node = null;
    @property(Node)
    play: Node = null;
    @property(Node)
    playText: Node = null;
    @property(Node)
    level1: Node = null;
    @property(Node)
    level1Text: Node = null;
    @property(Node)
    level2: Node = null;
    @property(Node)
    level2Text: Node = null;
    @property(Node)
    hint: Node = null;
    clickPlay(e) {
        console.log("PLAY")
        this.title.active = false;
        this.play.active = false;
        this.playText.active = false;
        this.level1.active = true;
        this.level1Text.active = true;
        this.level2.active = true;
        this.level2Text.active = true;
        // this.hint.active = true;
    }
    clickLevel1() {
        director.loadScene("scene");
    }
    clickLevel2() {
        director.loadScene("scene2");
    }
}


