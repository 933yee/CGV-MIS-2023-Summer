import { _decorator, Component, Vec3, systemEvent, SystemEventType, EventKeyboard, macro, KeyCode, v2, v3, RigidBody2D, Vec2, log, PhysicsSystem2D, Contact2DType, Camera, BoxCollider2D, Animation, Node, UI, RichText, Prefab, instantiate, AudioClip, AudioSource, director, game } from 'cc';
const { ccclass, property } = _decorator;
const Input = {};

@ccclass('playerControl')
export class playerControl extends Component {
    @property(Camera)
    camNode: Camera = null;
    @property(RichText)
    score: RichText = null;
    @property(RichText)
    health: RichText = null;
    @property(RichText)
    timer: RichText = null;
    @property(Number)
    moveSpeed: number = 200;
    @property(Number)
    jumpForce: number = 500;
    @property(Node)
    spawnPoint: Node = null;
    @property(Node)
    star: Node = null;
    @property(Prefab)
    score100Prefab: Prefab = null;
    @property(Prefab)
    score200Prefab: Prefab = null;
    @property(Prefab)
    score400Prefab: Prefab = null;
    @property(AudioClip)
    coinClip: AudioClip = null;
    @property(AudioClip)
    deadClip: AudioClip = null;
    @property(AudioClip)
    jumpClip: AudioClip = null;
    @property(AudioClip)
    stompClip: AudioClip = null;
    @property(AudioClip)
    brickClip: AudioClip = null;
    @property(AudioSource)
    bgm: AudioSource = null;

    private isLoadingMenu: boolean = false;
    private isLoadingLose: boolean = false;
    private isLoadingNext: boolean = false;
    private isJumping: boolean = false;
    private rigidbody: RigidBody2D = null;
    private playerAnimation: Animation = null;
    private playerAnimationText: string = null;
    private isDead: boolean = null;
    private healthInt: number = 3;
    private timerInt: number = 50;
    private scoreInt: number = 0;
    private isDying: boolean = false;

    onLoad() {
        Input[KeyCode.ESCAPE] = 0;
        Input[KeyCode.ARROW_LEFT] = 0;
        Input[KeyCode.ARROW_RIGHT] = 0;
        Input[KeyCode.SPACE] = 0;
        this.schedule(this.setTimer.bind(this), 1)
        this.score.string = "0";
        this.playerAnimation = this.getComponent(Animation);
        this.playerAnimationText = 'playerIdle';
        PhysicsSystem2D.instance.enable = true;
        this.rigidbody = this.getComponent(RigidBody2D);
        PhysicsSystem2D.instance.on(Contact2DType.BEGIN_CONTACT, this.onBeginContact, this);
        systemEvent.on(SystemEventType.KEY_DOWN, this.onKeyDown, this);
        systemEvent.on(SystemEventType.KEY_UP, this.onKeyUp, this);
        // console.log(this.getComponent(BoxCollider2D))
    }

    setTimer() {
        this.timerInt--;
        if (this.timerInt == 0) {
            this.dying();
            return;
        }
    }

    onDestroy() {
        systemEvent.off(SystemEventType.KEY_DOWN, this.onKeyDown, this);
        systemEvent.off(SystemEventType.KEY_UP, this.onKeyUp, this);
    }

    onKeyDown(event: EventKeyboard) {
        Input[event.keyCode] = 1;
    }

    onKeyUp(event: EventKeyboard) {
        Input[event.keyCode] = 0;
    }

    moveLeft() {
        this.node.scale = v3(-1, this.node.scale.y);
        this.rigidbody.linearVelocity = v2(-this.moveSpeed, this.rigidbody.linearVelocity.y);
    }

    moveRight() {
        this.node.scale = v3(1, this.node.scale.y);
        this.rigidbody.linearVelocity = v2(this.moveSpeed, this.rigidbody.linearVelocity.y);
    }

    stopMoving() {
        this.rigidbody.linearVelocity = v2(this.rigidbody.linearVelocity.x * 0.6, this.rigidbody.linearVelocity.y);
    }

    jump() {
        if (!this.isJumping) {
            this.jumpClip.play();
            this.isJumping = true;
            this.rigidbody.applyLinearImpulse(new Vec2(0, this.jumpForce), new Vec2(this.node.position.x, this.node.position.y), true);
        }
    }

    onBeginContact(self, other) {
        // console.log(other.node._name);
        if (other.node._name == 'star' || self.node._name == 'star') {
            director.loadScene("win");
        }
        if ((self.node._name == 'player' && other.tag == 0 && other.node._name != 'coin') || other.node._name == 'player' && self.tag == 0 && self.node._name != 'coin') {
            this.isJumping = false;
        }
        if ((self.node._name == 'player' && other.node._name == 'goomba' && other.tag == 0 || self.node._name == 'goomba' && other.node._name == 'player' && self.tag == 0) && !this.isDying) {
            this.dying();
        }
        if ((self.node._name == 'player' && other.node._name == 'goomba' && other.tag == 2)) {
            other.node.getComponent(Animation).play("goombaDie");
            this.rigidbody.applyLinearImpulse(new Vec2(0, this.jumpForce * 0.7), new Vec2(this.node.position.x, this.node.position.y), true);
            this.stompClip.play();
            other.node.getComponent(RigidBody2D).linearVelocity = v2(0, 0);
            let addScore = instantiate(this.score400Prefab);
            addScore.setPosition(new Vec3(0, 16, 0));
            addScore.parent = other.node;
            this.scoreInt += 400;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
                other.node.setPosition(-1000, -1000);
            }, 1)
        }
        if (self.node._name == 'goomba' && other.node._name == 'player' && self.tag == 2) {
            self.node.getComponent(Animation).play("goombaDie");
            this.rigidbody.applyLinearImpulse(new Vec2(0, this.jumpForce * 0.7), new Vec2(this.node.position.x, this.node.position.y), true);
            this.stompClip.play();
            self.node.getComponent(RigidBody2D).linearVelocity = v2(0, 0);
            let addScore = instantiate(this.score400Prefab);
            addScore.setPosition(new Vec3(0, 16, 0));
            addScore.parent = self.node;
            this.scoreInt += 400;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
                self.node.setPosition(-1000, -1000);
            }, 1)
        }


        if ((other.node._name == 'flower' && self.node._name == 'player' || other.node._name == 'player' && self.node._name == 'flower') && !this.isDying) {
            this.dying();
        }
        if ((other.node._name == 'respawn' && self.node._name == 'player' || self.node._name == 'respawn' && other.node._name == 'player') && !this.isDying) {
            this.dying();
        }
        if (other.node._name == 'respawn-deep' && self.node._name == 'player' || self.node._name == 'respawn-deep' && other.node._name == 'player') {
            this.isDying = false;
            this.die();
        }

        if (other.node._name == 'question' && self.node._name == 'player' && other.tag == 2) {
            if (!other.getComponent(Animation)._nameToState.question._isPlaying) return;
            this.coinClip.play();
            other.getComponent(Animation).play("questionUsed");
            let addScore = instantiate(this.score400Prefab);
            addScore.setPosition(new Vec3(0, 16, 0));
            addScore.parent = other.node;
            this.scoreInt += 400;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
            }, 1)
        }
        if (self.node._name == 'question' && other.node._name == 'player' && self.tag == 2) {
            if (!self.getComponent(Animation)._nameToState.question._isPlaying) return;
            this.coinClip.play();
            self.getComponent(Animation).play("questionUsed");
            let addScore = instantiate(this.score400Prefab);
            addScore.setPosition(new Vec3(0, 16, 0));
            addScore.parent = self.node;
            this.scoreInt += 400;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
            }, 1)
        }
        if (other.node._name == 'coin' && self.node._name == 'player') {
            this.coinClip.play();
            let addScore = instantiate(this.score200Prefab);
            addScore.setPosition(other.node.position);
            addScore.parent = other.node.parent;
            this.scoreInt += 200;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
            }, 1)
            other.node.active = false;
        }
        if (self.node._name == 'coin' && other.node._name == 'player') {
            this.coinClip.play();
            let addScore = instantiate(this.score200Prefab);
            addScore.setPosition(self.node.position);
            addScore.parent = self.node.parent;
            this.scoreInt += 200;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
            }, 1)
            self.node.active = false;
        }

        if ((other.node._name == 'brick' && self.node._name == 'player' && other.tag == 2)) {
            this.brickClip.play();
            this.scheduleOnce(() => {
                this.brickClip.stop();
            }, 0.3)
            let addScore = instantiate(this.score100Prefab);
            addScore.setPosition(other.node.position);
            addScore.parent = other.node.parent;
            this.scoreInt += 100;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
            }, 1)
            other.node.active = false;
        }
        if ((self.node._name == 'brick' && other.node._name == 'player' && self.tag == 2)) {
            this.brickClip.play();
            this.scheduleOnce(() => {
                this.brickClip.stop();
            }, 0.3)
            let addScore = instantiate(this.score100Prefab);
            addScore.setPosition(self.node.position);
            addScore.parent = self.node.parent;
            this.scoreInt += 100;
            this.score.string = this.scoreInt.toString();
            this.scheduleOnce(() => {
                addScore.destroy();
            }, 1)
            self.node.active = false;
        }
        // if(other)
    }

    setAnimation(animation) {
        if (this.playerAnimationText == animation) return;
        this.playerAnimationText = animation;
        // console.log(this.playerAnimationText);
        this.playerAnimation.play(this.playerAnimationText);
    }

    dying() {
        this.bgm.stop();
        this.deadClip.play();
        this.isDying = true;
        this.setAnimation("playerDie");
        this.rigidbody.linearVelocity = v2(0, 0);
        this.rigidbody.applyLinearImpulse(new Vec2(0, this.jumpForce * 1.5), new Vec2(this.node.position.x, this.node.position.y), true);
        // console.log(this.node.getComponent(BoxCollider2D).group);
        this.node.getComponent(BoxCollider2D).group = 16;
        this.healthInt--;
        this.health.string = this.healthInt.toString();
    }

    die() {
        this.node.getComponent(BoxCollider2D).group = 1;
        this.setAnimation("playerIdle");
        this.isDead = true;

        if (this.healthInt == 0) {
            this.isLoadingLose = true;
            director.loadScene("lose");
        } else {
            this.bgm.play();
        }
        this.timerInt = 50;
        this.timer.string = this.timerInt.toString();
    }

    protected update(dt: number): void {
        if (this.isLoadingMenu || this.isLoadingLose) return;
        if (Input[KeyCode.ESCAPE]) {
            this.isLoadingMenu = true;
            director.loadScene("menu");
            return;
        }

        let anima = this.playerAnimationText;
        let targetPos = new Vec3(this.node.getPosition().x, this.camNode.node.getPosition().y, 0);
        this.camNode.node.setPosition(targetPos);
        // let scoreTargetPos = new Vec3(targetPos.x, targetPos.y, 0);
        // this.score.node.setPosition(targetPos);
        if (this.isDying) return;

        if (this.isDead) {
            this.node.setPosition(this.spawnPoint.getPosition());
            this.isDead = false;
        }
        if (this.scoreInt >= 10000)
            this.star.active = true;

        if (Input[KeyCode.ARROW_LEFT]) {
            this.moveLeft();
            anima = 'playerRun';
        }
        else if (Input[KeyCode.ARROW_RIGHT]) {
            this.moveRight();
            anima = 'playerRun';
        }
        else {
            this.stopMoving();
            anima = 'playerIdle';
        }

        if (Input[KeyCode.SPACE]) this.jump();


        if (this.isJumping) anima = 'playerJump';

        if (anima) {
            this.setAnimation(anima);
        }
        // console.log(anima);
        this.timer.string = this.timerInt.toString();
    }
}


