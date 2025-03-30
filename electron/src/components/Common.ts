
import fs from "fs";
import { Toast } from '@douyinfe/semi-ui';
import i18n from 'i18next';
import {osLocale} from 'os-locale';
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();
import * as dlg from "../components/Dialog";
import { yxulinux_connect_state, yxulinux_error_code as cc } from "../types/ErrorCode";





/**
 * 设置窗口CSS样式
 * @param ele 挂载的HTML节点
 * @param label 窗口置顶类型：remove_dialog 移除其它窗口; modify_zindex 修改其它窗口Z轴顺序，将当前窗口置顶显示; 
 */
export function setdlgcss(ele: HTMLDivElement, label: string) {
  if (label == "remove_dialog") {
    // 移除其它窗口
    let dlgs: HTMLCollectionOf<Element> = document.getElementsByClassName("dlg");
    if (dlgs.length > 0) {
      for (let i = 0; i < dlgs.length; i++) {
        let el = dlgs.item(i) as HTMLDivElement;
        document.body.removeChild(el);
      }
    }
  }
  else if (label == "modify_zindex") {
    // 修改z轴顺序
    let dlgs: HTMLCollectionOf<Element> = document.getElementsByClassName("dlg");
    if (dlgs.length > 0) {
      for (let i = 0; i < dlgs.length; i++) {
        let el = dlgs.item(i) as HTMLDivElement;
        el.style.zIndex = 9 + '';
      }
    }
  }
  // 将当前窗口置顶
  ele.className = "dlg";
  ele.style.zIndex = 10 + '';
  ele.style.position = "absolute";
  ele.style.left = '0px';
  ele.style.top = '0px';
}




export async function setlocallangini(lang: string) {
  let langini = apphandle.appinfo.vilshelldir + global.DIRSEPA + "uilang.ini";
  apphandle.writefile(langini, lang);
}



export function langs() {
  return [
    { value: 'zh_cn', label: '简体中文', otherKey: 0 },
    { value: 'zh_hk', label: '繁體中文', otherKey: 1 },
    { value: 'en_us', label: 'English', otherKey: 2 }
  ];
}

export function getlang(lang: string) {
  let _langs = langs();
  let _lang;
  for (let i = 0; i < _langs.length; i++) {
    let ll = _langs[i];
    if (ll.value == lang) {
      _lang = ll;
      break;
    }
  }
  return _lang;
}




/**
 * 时间戳转日期
 * @param timestamp 
 * @param type 
 * @returns 
 */
export function dateTranslate1(timestamp: number, type: number = 1) {
  let date = new Date(timestamp * 1000);  // 参数需要毫秒数，所以这里将秒数乘于 1000
  if (date == null) return ''
  let Y = date.getFullYear();
  let M = (date.getMonth() + 1 < 10 ? '0' + (date.getMonth() + 1) : date.getMonth() + 1);
  let D = (date.getDate() < 10 ? '0' + (date.getDate()) : date.getDate());
  let h = date.getHours() >= 10 ? date.getHours() + '' : '0' + date.getHours();
  let m = date.getMinutes() >= 10 ? date.getMinutes() + '' : '0' + date.getMinutes();
  let s = date.getSeconds() >= 10 ? date.getSeconds() + '' : '0' + date.getSeconds();
  let W = date.getDay();

  // 星期数组
  const weekArr = ['日', '一', '二', '三', '四', '五', '六'];
  let wstr = `周${weekArr[date.getDay()]}`
  // 获取传入日期与当前日期相差的天数，一天 86400000 毫秒
  const diffTime = (date.setHours(0, 0, 0, 0) - new Date().setHours(0, 0, 0, 0)) / 86400000;
  const dayArr = ['前天', '昨天', '今天', '明天', '后天'];
  // 获取“前天/昨天/今天/明天/后天”，若不在这些里面则显示星期
  let label = dayArr[diffTime + 2] || '';
  if (label) {
    label = `${wstr} ${label}`
  } else {
    label = wstr;
  }

  if (type == 1) {
    return `${Y}-${M}-${D}`;
  }
  else if (type == 2) {
    return `${M}月${D}日`
  }
  else if (type == 3) {
    return label
  }
  else if (type == 4) {
    return `${M}月${D}日 ${label}`
  }
  else if (type == 5) {
    return `${Y}-${M}-${D} ${h}:${m}:${s}`;
  }
  else if (type == 6) {
    return `${h}:${m}`;
  }
  else if (type == 7) {
    return `${M}-${D} ${h}:${m}`;
  }
}






/**
 * 复制文本到剪切板
 * @param text 
 */
export function copyTextToClipboard(text: string): Promise<Boolean> {
  return navigator.clipboard.writeText(text).then(
    () => {
      console.log('文本已成功复制到剪切板');
      Toast.info(i18n.t('Copied'));
      return true;
    },
    (err) => {
      console.error('无法复制文本到剪切板: ', err);
      return false;
    }
  );
}




export function isConnHost(){
  if (global.vilssh == null) {
    return false;
  }else{
    if(global.vilssh.conn!=yxulinux_connect_state.YXU_CONNECT_AUTH){
      return false;
    }
  }
  return true;
}


/**
 * 成功连接之后
 */
export async function afterSuccessfulConn(){
  await global.vilssh.afterSuccessfulConnection();
}



/**
 * 关闭连接
 */
export function closeConnReleaseConn(){
  if(global.vilssh){
    global.vilssh.releaseconn();
    global.vilssh = null;
    //
    history.replace({pathname: "/launchhome"});
    dlg.dlg_close_all();
    global.ievt.emit("sethoststring", "");
  }
}



//文件大小单位转换
export function renderSize (value: any) {
  if (null == value || value == '') {
    return "0 Bytes";
  }
  var unitArr = new Array("Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB");
  var index = 0;
  var srcsize = parseFloat(value);
  index = Math.floor(Math.log(srcsize) / Math.log(1024));
  var size: any = srcsize / Math.pow(1024, index);
  size = size.toFixed(2);//保留的小数位数
  return size + unitArr[index];
}




export async function getlocallangini() {
  let langini = apphandle.appinfo.vilshelldir + global.DIRSEPA + "uilang.ini";
  // 如果语言配置文件不存在
  if (!fs.existsSync(langini)) {
    const locale = await osLocale();
    if(locale=='zh-CN'){
      return 'zh_cn'
    }else{
      return "en_us";
    }
  } else {
    let content = fs.readFileSync(langini).toString("utf8");
    return content;
  }
}