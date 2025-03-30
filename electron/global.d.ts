import indexevt from "./src/indexevt"




declare global {
  var vildoc: any;
  var mainWindow: any;
  var vilssh: any;
  var apphandle: any;
  var uilang: any;
  var savestate1 : any;
  var elec: any;
  var DIRSEPA: any;
  var alert2:any;
  var confirm2:any;
  var toast2:any;
  var ievt:indexevt;
  interface timeout {
    reconn_time: NodeJS.Timeout;
    reauth_time: NodeJS.Timeout;
  }
  var useruuid: string;
}

export {};