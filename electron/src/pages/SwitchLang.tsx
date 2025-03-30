
import * as React from 'react'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import * as ReactDOM from 'react-dom'
import i18n from 'i18next';
//-------------------------------
import * as Common from "../components/Common";
import * as dlg from "../components/Dialog";
import { Switch, RadioGroup, Radio, Tabs, TabPane } from '@douyinfe/semi-ui';

import "./SwitchLang.css"
import villib1 from "@/../dist/villib1.node";



type IProps = PropsWithChildren<{
  onClose: () => void
}>;
class SwitchLang1 extends React.Component<IProps, any>{
  constructor(props: IProps) {
    super(props);
    this.onsave = this.onsave.bind(this);
  }
  state = {
    "uilang": global.uilang,
    "zh_cn": "1",
    "en_us": "1",
    "zh_hk": "1",
    "de": "0",
    "fr": "0",
    "ru": "0",
    "ja": "0",
    "ko": "0",
    "th": "0",
    "ar": "0",
    "pt": "0",
    "tr": "0",
    "es": "0",
    "ur": "0",
    "vi": "0",
    "ms": "0",
    "id": "0"
  }
  componentDidMount(): void { }
  onsave() {
    try {
      dlg.alert(i18n.t('tips'), i18n.t('Successfully set, re-open the software to take effect.'), 
      ()=>{
        Common.setlocallangini(this.state.uilang);
        this.props.onClose();
      });
    } catch (error) {
      // console.error("切换失败", error);
      dlg.alert(i18n.t('error'), i18n.t('api_1024'));
    }
  }
  render(): React.ReactNode {
    return (
      <div className='SwitchLang1'>
        <div className='SwitchLang1_bg'></div>
        <div className='SwitchLang1_win'>
          <div className='SwitchLang1_close'><a onClick={() => this.props.onClose()}><i className='iconfont'>&#xe8bb;</i></a></div>
          <div className='SwitchLang1_c'>
            <div className='title'>{i18n.t('SwitchLang')}</div>
            <RadioGroup value={this.state.uilang} onChange={e => this.setState({ uilang: e.target.value })}>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.zh_cn == "1" ? false : true} value="zh_cn">简体中文</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.zh_hk == "1" ? false : true} value="zh_hk">繁體中文</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.en_us == "1" ? false : true} value="en_us">English</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.de == "1" ? false : true} value="de">Deutsch</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.fr == "1" ? false : true} value="fr">Français</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.ru == "1" ? false : true} value="ru">Русский</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.ja == "1" ? false : true} value="ja">日本語</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.ko == "1" ? false : true} value="ko">한국어</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.th == "1" ? false : true} value="th">แบบไทย</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.ar == "1" ? false : true} value="ar">عربي</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.pt == "1" ? false : true} value="pt">Português</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.tr == "1" ? false : true} value="tr">Türkçe</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.es == "1" ? false : true} value="es">español</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.ur == "1" ? false : true} value="ur">اردو</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.vi == "1" ? false : true} value="vi">Tiếng Việt</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.ms == "1" ? false : true} value="ms">Melayu</Radio>
              </div>
              <div className='SwitchLang1_g'>
                <Radio disabled={this.state.id == "1" ? false : true} value="id">bahasa Indonesia</Radio>
              </div>
            </RadioGroup>
            <div className='h20'></div>
            <a className='SwitchLang1_d' onClick={this.onsave}>{i18n.t('Change')}</a>
          </div>
        </div>
      </div>
    )
  }
}

const closeSwitchLang1dlg = () => {
  try {
    ReactDOM.unmountComponentAtNode(document.getElementById('SwitchLang1dlg'));
    document.body.removeChild(document.getElementById('SwitchLang1dlg'));
  } catch (error) {
    console.error(error);
  }
}

const SwitchLang1dlg = (label = 'modify_zindex') => {
  try {
    let _exists = document.getElementById('SwitchLang1dlg');
    if (_exists) {
      Common.setdlgcss(_exists as HTMLDivElement, 'modify_zindex');  // 将窗口置顶
      return;
    }
    let ele = document.createElement("div");
    ele.id = "SwitchLang1dlg";
    Common.setdlgcss(ele, label);  // 将窗口置顶
    document.body.appendChild(ele);
    ReactDOM.render(<SwitchLang1 onClose={closeSwitchLang1dlg} />, ele);
  } catch (error) {
    console.error(error);
  }
}

export { SwitchLang1dlg, closeSwitchLang1dlg }

export default SwitchLang1;