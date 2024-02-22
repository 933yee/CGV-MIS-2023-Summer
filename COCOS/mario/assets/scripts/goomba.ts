import { _decorator, Animation, BoxCollider2D, Component, Node, PhysicsSystem2D, RigidBody2D, v2 } from 'cc';
const { ccclass, property } = _decorator;

@ccclass('goomba')
export class goomba extends Component {
    update() {
        if (!this.getComponent(Animation).getState("goombaDie").isPlaying)
            this.getComponent(RigidBody2D).linearVelocity = v2(-3, this.getComponent(RigidBody2D).linearVelocity.y);
        else {
            this.getComponent(BoxCollider2D).enabled = false;
            this.getComponent(RigidBody2D).enabled = false;
        }
    }
}


