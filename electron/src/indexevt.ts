import EventEmitter from "events";
class indexevt extends EventEmitter {
  constructor() {
    super();
    this.setMaxListeners(512);
  }
}
export default indexevt;