import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import enUS from './en_us.json';
import zhCN from './zh_cn.json';
import zhTW from './zh_hk.json';
import * as Common from "../components/Common"

const resources = {
	'en_us': {
		translation: enUS,
	},
	'zh_cn': {
		translation: zhCN,
	},
	'zh_hk': {
		translation: zhTW,
	}
};

function loadlang(){
	return Common.getlocallangini().then((_lang)=>{
		global.uilang =_lang;
		i18n.use(initReactI18next).init({
			resources,
			lng: _lang,
			interpolation: {
				escapeValue: false,
			},
		});
	});
}

//i18n.changeLanguage('en_us')

export { loadlang }
export default i18n;