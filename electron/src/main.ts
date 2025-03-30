import { IpcMainInvokeEvent, IpcMainEvent } from "electron"
import { app, BrowserWindow, BrowserView, ipcMain, dialog, webContents, session, shell, Menu } from "electron";
var path = require("path");
var fs = require("fs");
const villib1 = require("../dist/villib1.node");
const apphandle = new villib1.apphandle();

var DIRSEPA="";
var PLATFORM="";
var _path = "";


if(process.platform=='darwin'){
  DIRSEPA='/';
  PLATFORM='mac';
  _path = app.getAppPath()+DIRSEPA+"dist";
}
else if(process.platform=='win32'){
  DIRSEPA='\\';
  PLATFORM='win';
  _path = app.getAppPath()+DIRSEPA+"dist";
}
else if(process.platform=='linux'){
  DIRSEPA='/';
  PLATFORM='linux';
  _path = app.getAppPath()+DIRSEPA+"dist";
}

///===============================================================

if (process.env.NODE_ENV == "development") {
  console.log("electron version: ", app.getVersion());
  console.log("chrome version: ", process.versions.chrome);
  console.log("V8 version: ", process.versions.v8);
  console.log("Node version: ", process.versions.node);
  console.log("arch: ", process.arch);
  console.log('home: ', app.getPath("home"));
  console.log('appData: ', app.getPath("appData"));
  console.log('userData: ', app.getPath("userData"));
  console.log('temp: ', app.getPath("temp"));
  console.log('exe: ', path.dirname(app.getPath("exe")));
  console.log('dir: ', path.dirname(__dirname));
  console.log('getAppPath: ', app.getAppPath());
  console.log('Running in 32-bit mode:', process.arch);
  _path = __dirname;
}else{
}


// 解决苹果和linux窗口不显示的问题
function mac_win_bug(){
  if(process.platform=='darwin'||process.platform=='linux'){
    let [ww, hh] = global.mainWindow.getSize();
    global.mainWindow.setSize(ww+1, hh+1);
  }
}


function _ipchandle() {
  //
  ipcMain.handle("vildoc.closewin", () => app.quit());
  ipcMain.handle("vildoc.minwin", () => global.mainWindow.minimize());
  // 窗口最大化
  ipcMain.handle("vildoc.maxwin", (event:any, arg:any) => {
    if (global.mainWindow.isMaximized()) {
      global.mainWindow.unmaximize()
    } else {
      global.mainWindow.maximize()
    }
  });
  // 恢复窗口
  ipcMain.handle("vildoc.restore", (event:any, arg:any) => {
    global.mainWindow.restore()
    global.mainWindow.focus()
    if (global.mainWindow.isMaximized()) {
      global.mainWindow.unmaximize()
    }else{
      global.mainWindow.maximize()
      global.mainWindow.unmaximize()
    }
  });
  // 保存文件
  ipcMain.handle("vildoc.savefile", (event:any, arg:any) => {
    let options = {
      title: arg,
      defaultPath: app.getPath("home"),
    };
    let strings = dialog.showSaveDialogSync(global.mainWindow, options);
    return strings;
  });
  // 选择文件夹
  ipcMain.handle("vildoc.choosedirectory", (event:any, arg:any) => {
    let options:any = {
      title: arg,
      defaultPath: app.getPath("home"),
      properties: ['openDirectory']
    };
    let strings = dialog.showOpenDialogSync(global.mainWindow, options);
    return strings;
  });
  // 选择文件
  ipcMain.handle("vildoc.choosefile", (event:any, arg:any) => {
    let options:any = {
      title: arg,
      defaultPath: app.getPath("home"),
      properties: ['openFile']
    };
    let strings = dialog.showOpenDialogSync(global.mainWindow, options);
    return strings;
  });
  // 获取HOME目录
  ipcMain.handle("vildoc.gethomepath", (event:any, arg:any) => {
    console.log('home', app.getPath("home"));
    console.log('appData', app.getPath("appData"));
    console.log('userData', app.getPath("userData"));
    console.log('temp', app.getPath("temp"));
    console.log('exe', app.getPath("exe"));
    return app.getPath(arg);
  });
  // 跳转URL
  ipcMain.handle("vildoc.jumpurl", (event:any, arg:any) => {
    let url = arg;
    shell.openExternal(url);
  });
  // 打开开发工具栏
  ipcMain.on('openDevtools', (event:any, arg:any) => {
    if (global.mainWindow.webContents.isDevToolsOpened()) {
      global.mainWindow.webContents.closeDevTools()
    } else {
      global.mainWindow.webContents.openDevTools()
    }
  });
  // 阻塞式弹窗
  ipcMain.handle("vildoc.showMessageBoxSync", (event:any, arg:any) => {
    const {message,type,buttons,title} = arg;
    return dialog.showMessageBoxSync(global.mainWindow, {message,type,buttons,title});
  });
  ipcMain.handle("vildoc.onfocus", (event:any, arg: any) => {
    app.focus();
    return "";
  });
  ipcMain.handle("vildoc.mac_win_bug", (event:any, arg: any) => {
    mac_win_bug();
    return true;
  });
  ///
  ipcMain.handle("vildoc.handle_test", (event:any, arg: any) => {
    return "handle_test";
  });
  ipcMain.on("vildoc.on_test", (event: IpcMainEvent, arg: any) => {
    event.returnValue = "on_test";
  });
  // ...
};


const createmenu = ()=>{
  // global.elec_Menu 
  const isMac = process.platform === 'darwin'
  const template:any = [
    // { role: 'appMenu' }
    ...(isMac
      ? [{
          label: "vilshell",
          submenu: [
            { role: 'about' },
            { role: 'quit' }
          ]
        }]
      : []),
    // { role: 'fileMenu' }
    {
      label: 'File',
      submenu: [
        isMac ? { role: 'close' } : { role: 'quit' }
      ]
    },
    // { role: 'editMenu' }
    {
      label: 'Edit',
      submenu: [
      ]
    },
    // { role: 'viewMenu' }
    {
      label: 'View',
      submenu: [
      ]
    },
    // { role: 'windowMenu' }
    {
      label: 'Window',
      submenu: [
      ]
    },
    {
      role: 'help',
      submenu: [
        {
          label: 'Help',
          click: async () => {
            await shell.openExternal('http://vilshell.com')
          }
        },
        {
          label: 'About',
          click: async () => {
            await shell.openExternal('http://vilshell.com')
          }
        }
      ]
    }
  ]
  var menu = Menu.buildFromTemplate(template);
  Menu.setApplicationMenu(menu);
}

const onReady = async ()=>{

  app.commandLine.appendSwitch("ignore-certificate-errors");

  _ipchandle();
  createmenu();

  global.mainWindow = new BrowserWindow({
    width: 1280,
    height: 720,
    minHeight: 700,
    minWidth: 1190,
    fullscreen: false,
    frame: false,
    // icon: path.join(_path, "logo.ico"),
    webPreferences: {
      nodeIntegration: true,
      nodeIntegrationInWorker: true,
      contextIsolation: false,
      zoomFactor: 1
    }
  });

  global.mainWindow.menuBarVisible = false;  //隐藏菜单
  if (process.env.NODE_ENV == "development") {
    global.mainWindow.loadURL(`http://127.0.0.1:8080/#/launchhome`);
    let wwebc = global.mainWindow.webContents;
    wwebc.openDevTools();  // 打开开发工具
  } 
  else 
  {
    global.mainWindow.loadFile("dist/index.html", {hash:"/launchhome"});
  }

  global.mainWindow.on("ready-to-show", ()=>{
    // mainWindowSize = global.mainWindow.getSize();
  })
  global.mainWindow.on("resize", ()=>{
    // mainWindowSize = global.mainWindow.getSize();
  })

};


app.on("ready", onReady);