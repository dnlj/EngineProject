#pragma once

#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <random>


namespace Bench {
	// New seed: std::cout << "\n\n"; for (int i = 0; i < 128; ++i) { std::cout << "0x"<< std::hex << std::uppercase << std::random_device{}() << "u, "; } std::cout << "\n\n";
	inline const std::seed_seq seeds[] = {
		{0x75106832u, 0x2397B5A1u, 0x457936Eu, 0x234FA80u, 0xA9E39834u, 0x39E0162Fu, 0x1A96E20Eu, 0xAE0535CEu, 0xD01CC323u, 0xFA9D9792u, 0x645A3124u, 0x1E460FE8u, 0xD5F02B3Au, 0x2D8CA1Eu, 0xD2A3B31Cu, 0x828F785Du, 0x23BA589u, 0xE09221BDu, 0x431149B7u, 0x5CFC1755u, 0x9F6AFABDu, 0xF6AD9458u, 0xCCD1C372u, 0x7A81711Cu, 0xF393944Cu, 0xEF99DF6Au, 0x8F120DEu, 0xF46CCCF3u, 0x980CCF5Fu, 0xF041DFA9u, 0xE64A0094u, 0xDB8BAA5Eu, 0x74BAC338u, 0x6D797465u, 0xEFE00816u, 0x5263D42u, 0x6D5825B3u, 0x651774Fu, 0x98EBC809u, 0x4ED6CB1Cu, 0xFA3D0A13u, 0x568B773Bu, 0x4FA7D4F1u, 0xC0C534F5u, 0x4D4D766Du, 0xB5CA6600u, 0x4F6CA519u, 0xD74B2B6u, 0xB09E7948u, 0x2FFB60C2u, 0x650E4841u, 0xAC69311Fu, 0x4484CBE3u, 0x141B1C0Bu, 0x641066CEu, 0xBB35C31Cu, 0xBD407F50u, 0x3B9610EFu, 0xB057FFB2u, 0x2134BF52u, 0x9FE24508u, 0xE36B9549u, 0x633AA7EEu, 0x9ECC66EBu, 0xF17E6C82u, 0xAA4B3C59u, 0xA3AED572u, 0xD6DAFFD1u, 0xAEECEAF7u, 0xBCA1ECA5u, 0x16CAC278u, 0x4E5DC2C6u, 0xF267F8A2u, 0x7B58A048u, 0xA585796Bu, 0x9964B522u, 0x580AA980u, 0x89339A46u, 0x580FFDC3u, 0xD6C69DF0u, 0xA559C9B9u, 0x461E655Au, 0xD277DB0Eu, 0x4A343129u, 0xFCF4AB2Bu, 0x4324F26Au, 0xC510BCFDu, 0xAD3E9A22u, 0x4C8D227Du, 0x27E77384u, 0x2E57E62Fu, 0x930E2DD0u, 0x6E56AEE8u, 0x6D976138u, 0x412FCA68u, 0x298156CEu, 0x3584A5ABu, 0x4BA527DDu, 0x8AB4C69Bu, 0x736E6F16u, 0xC2164BEFu, 0xB1CE39E3u, 0x2FE61036u, 0x30AD8892u, 0x7DF2DA23u, 0xA0F0EE0Au, 0xEA0DBFC3u, 0x48555391u, 0x8B600E4Du, 0xDDC8B4ADu, 0xDAD019FAu, 0xB0FBD8BAu, 0xAC7899E2u, 0x6CD08CF6u, 0x9CEE0B84u, 0x900AD621u, 0xBDAF4062u, 0xF86589DAu, 0x24FA873Eu, 0xDB53A536u, 0x54C29AE9u, 0x898A1CB6u, 0x5CF366A4u, 0xA1ECE5ECu, 0x1727AC3Fu, 0x374E648Fu, 0x826EBC76u, 0x4B499542u},
		{0x82F4ECD9u, 0x61CB15C0u, 0x111BB200u, 0x1FEB0BADu, 0x7D4FAB95u, 0x862C2A31u, 0x4F822AAFu, 0x9C0B4978u, 0x3D697A15u, 0xF5B7D7F1u, 0xF3B7DD49u, 0xE9C6EEB3u, 0x92402907u, 0x6AD57D2Cu, 0x5BB370B7u, 0xCF4BBA52u, 0xEC475641u, 0xEEF13362u, 0xE462D699u, 0x1F9CE906u, 0x215F8CBDu, 0x9BD8DAE8u, 0xDCD04ECFu, 0xFAE294FBu, 0x5B63580u, 0x12536273u, 0x4A14637Eu, 0x95728ECDu, 0xA8169B86u, 0xB08EC270u, 0xF25EF1B4u, 0x48543F68u, 0x1503E08Du, 0x994BE83Du, 0xDE97ABC1u, 0x7775A34Cu, 0xBCA46FD4u, 0x18C25E8Fu, 0x67EA114Eu, 0xAA07097Cu, 0xB3FC4780u, 0x831CA95Bu, 0xD47D4DEBu, 0x5C7B0E8Fu, 0x429DCCA1u, 0x5BE31C6Bu, 0x27F98594u, 0xD1090923u, 0xAA9BB35Au, 0x4430D985u, 0xE0123093u, 0x52328249u, 0xBC1C3563u, 0x213190D8u, 0x1810D4A2u, 0x49174DE2u, 0xB8A44A4Cu, 0x48B31A12u, 0xEF50A047u, 0x50A90EB5u, 0xCA7F22A5u, 0xA1C35933u, 0x1FDFD785u, 0x870D986Au, 0xAB0F0681u, 0x4EBD941u, 0x4C9922DDu, 0xE047E08Bu, 0xC9630264u, 0x2F4DAD6Au, 0x35851A4Bu, 0xF1DDF01Du, 0x375B434Au, 0xD3F89B89u, 0xF27B1199u, 0x45D31115u, 0x9EBEDE0Cu, 0x634793EEu, 0xDA501DB6u, 0x695EBE6Eu, 0xDE6D7486u, 0x463A402Au, 0x8AB7DBF1u, 0x5FF3F548u, 0xF39E144Fu, 0xDAFD151Cu, 0xFF493EA8u, 0xBD03254Cu, 0x65AAF473u, 0x6FD95076u, 0xF7F4BE08u, 0x69D9742Fu, 0x95559082u, 0x8BAE34C1u, 0x784275D8u, 0x4B9ACB8Au, 0x15A1CE5u, 0x8CCEA21Du, 0xAD81BE2Eu, 0xC3056062u, 0xB765253Cu, 0x51486114u, 0xF67ECE0Eu, 0xED0B32u, 0x4C4D86EDu, 0xBA21EE00u, 0x415C482Fu, 0xDEC849E9u, 0x9BAA135Fu, 0x48A40472u, 0xB87057F2u, 0x98ADCC5Eu, 0x86E46A14u, 0xA921BC0u, 0x668507E6u, 0xB3166CD7u, 0x3162EACFu, 0x88A34F30u, 0x1D9682EAu, 0xF9C5F9D2u, 0xCC7579Bu, 0x2989AA90u, 0xA2D92068u, 0xE1418071u, 0xE3DDE4B5u, 0xD8FDF669u, 0x18D9A611u, 0xA46B250Bu,},
		{0x4F00D7AAu, 0xDE0227ABu, 0x524F6907u, 0x69A024DEu, 0xA7F00216u, 0xFB49533Bu, 0x75CE0584u, 0xC4FC9E0Fu, 0x3635E06Du, 0xD0629C1Du, 0x67196790u, 0xDCBDD1D7u, 0xB09D1D48u, 0x7173F71Cu, 0x9DC30918u, 0xFEE3B8B6u, 0x6102F61Cu, 0x6E100FF9u, 0xDB3E504Du, 0x83B5BE61u, 0xB301ED22u, 0x47C55C42u, 0xB604714Au, 0x9B96AE7Bu, 0x5E4ED0Cu, 0x58C309E1u, 0xF8337292u, 0x755D6D76u, 0xC4EE8674u, 0x5899C76Bu, 0x43B8562Au, 0xA8131DDu, 0x62FF8D52u, 0x82C94A3Au, 0x871FCADu, 0xDCDB4FC1u, 0x23308888u, 0x6D012674u, 0x7574636u, 0xFB809FDBu, 0x6C75485Fu, 0x7409C826u, 0xDD26302Cu, 0x42172511u, 0x2FB5B1C7u, 0xB98647B7u, 0x8D824EABu, 0xAA784953u, 0x6B765185u, 0x3F0ED736u, 0x7035C861u, 0x9FABC4CFu, 0xC7372BE4u, 0xF7E31374u, 0x1CC0E76u, 0x90ED91ABu, 0xBB3285ECu, 0xF89D828u, 0x7DF563B2u, 0x448537DFu, 0x5709A4B5u, 0xBE3EECB1u, 0x5402037Fu, 0x398016DFu, 0xDD20EC0Au, 0x94CCA1CDu, 0x6FD0C757u, 0x281524A6u, 0xC8AE3828u, 0x1880A1u, 0x77C153FAu, 0x11573682u, 0x20B1BBB4u, 0x8690794Bu, 0x191B465Au, 0xBFE0537Au, 0x6A2711D9u, 0xE93B9F17u, 0xEC660E03u, 0x977639FDu, 0x7F64D542u, 0x1E957E1Au, 0x1CA6BDDEu, 0xA9FCA621u, 0x8BD8EBAFu, 0x881FC65Au, 0xA2F4E3A6u, 0x20ECA0A8u, 0xFC216148u, 0xFD4D529Bu, 0x54B598A8u, 0x92844D2Bu, 0xC1A41AF1u, 0xDF5E109u, 0x738BD356u, 0x7FD8F514u, 0x54EA671u, 0x8A90DC22u, 0xB096E844u, 0x731EBC72u, 0xB3CE6D3u, 0xBE3D46D9u, 0x37D0129Bu, 0x57056A27u, 0x19B33A9Du, 0xCA8C294Fu, 0xACDCDA33u, 0xBA113A19u, 0x1C3AF732u, 0xD5D23935u, 0x8550DA00u, 0x7B9AA035u, 0xEFF082B6u, 0xE5C1886Fu, 0xE2300ADBu, 0x5CFBB3AEu, 0x3392A746u, 0x454A8914u, 0xCA3A161Eu, 0x615A9408u, 0x6FE6CA4Cu, 0xC73F9212u, 0x844E933u, 0x6B0EB8C2u, 0xDA0B67B8u, 0x4E63B59u, 0x6260E191u, 0xDFF7D702u,},
		{0x9947901Fu, 0x3821DFC7u, 0x37E90D15u, 0x46D738A4u, 0x30A428D3u, 0xE94AE5A4u, 0x48CA0A1Eu, 0x19C0E3EBu, 0x6CA39EBCu, 0xFD4B82CAu, 0xF590CE96u, 0xB3D596B4u, 0xC0116604u, 0xF6BDB13Cu, 0xD4EADCE5u, 0xEECA6B5Au, 0x1ED2EB62u, 0x70611BAEu, 0xB90A3C87u, 0xCFB932Eu, 0x226C9E63u, 0x4C676B15u, 0x57D71F48u, 0x9F4050FEu, 0xD17236Eu, 0xEFF79E70u, 0x95A020E8u, 0xA22F11DFu, 0x871F2AF3u, 0xD6CBCB22u, 0x712E3F1Fu, 0x2938B1F1u, 0x14687C57u, 0x1097D939u, 0x942C2BB1u, 0x9B01C17Du, 0x241DEEBu, 0xF14173Eu, 0xD5CA4872u, 0x3A510F3Fu, 0x389BC573u, 0x72ECF745u, 0x28E1751Fu, 0x8E625EDAu, 0x9FB21568u, 0xDFF83276u, 0xD7CC182Fu, 0xDE7CAD56u, 0x4E22D627u, 0x336227A6u, 0xBBF626D3u, 0xAB963BFFu, 0xFEA423Fu, 0x3C95DCCDu, 0x62484C03u, 0x1469428Du, 0x5F3A7DCCu, 0x8C321BD1u, 0x13F63EABu, 0x7A1D53D9u, 0x15BC3AECu, 0x5461190Au, 0x9046EECu, 0x8626F320u, 0x2CA0FEEEu, 0x28B02470u, 0x4E1A9081u, 0x6AA9002u, 0x1C3117B6u, 0xD2059DD3u, 0x3294CF50u, 0xC46FD025u, 0x29D8D167u, 0xE0C11033u, 0x2C2B623Fu, 0x9EF95755u, 0x32F71EC5u, 0x739AC7EDu, 0x8B454D24u, 0xE2D51457u, 0xB8C32B88u, 0x609718F5u, 0x1417EDC3u, 0x57B6DABFu, 0x9DB357B6u, 0x44F4CAC5u, 0x1FD26266u, 0xBF9E6E4Bu, 0xCB5B7556u, 0xB872834Au, 0xAAB875A6u, 0xD52EB7F4u, 0x13939A80u, 0x1513BC6u, 0xAA0E1691u, 0xC60E2E56u, 0x3A7125F7u, 0x8AB60CBu, 0xE8AF5FA1u, 0x6BFEC2D8u, 0x58CCA7D3u, 0xC136EA5u, 0x9CD6D825u, 0x16EDF15Bu, 0x414F349Bu, 0xCBB1EDu, 0x43C9B6ABu, 0xF514AD11u, 0x57C32F00u, 0xB2E8AAu, 0x39E33428u, 0x7ADD5302u, 0x7703CED8u, 0x9E5CED6Bu, 0x76D9ABD9u, 0xE6B7C6C6u, 0x804AA7AFu, 0x4A495BC1u, 0x349C8D38u, 0x58E33C34u, 0xC6915EA5u, 0xADC41D7Fu, 0xAB44DE86u, 0xEDB5D4DCu, 0xEB6CAEE7u, 0x214820A3u, 0xEA5ED80Cu, 0xF1B89E78u,},
		{0x5E6CBE0u, 0x2C83344Du, 0xB0CBCE60u, 0xABDA0C32u, 0xD90A2C06u, 0x2DDFBBD3u, 0x83043592u, 0xF76900E8u, 0x1BA14D52u, 0xCABC7DE9u, 0x370A6FE5u, 0x6F17997Cu, 0x89D2E1C7u, 0xB598F14u, 0x247A2381u, 0xF5EF2B53u, 0x5B55498Eu, 0xAFE01F9Cu, 0x8A5BA3B2u, 0xED0F78E6u, 0xC915C368u, 0x8FE4CFCBu, 0x3D1D4723u, 0x65785BEFu, 0x244EE0A6u, 0xC21A090Bu, 0x909487E4u, 0xA175BD97u, 0xD629F240u, 0x1E856E26u, 0xCA0FC605u, 0xB4D3D66Cu, 0xC9E4A191u, 0x2DCB7754u, 0xE81AAD3Fu, 0x2047EFD3u, 0x963C3C7Eu, 0x4E6AA92Fu, 0x286D207Du, 0x1C15E30Bu, 0x81A5E4E3u, 0xEF630400u, 0x7F991603u, 0xF3F6394Fu, 0xDB462E9Fu, 0xA3D42FD5u, 0x65A4150Bu, 0x3E46EF6Fu, 0x1C9E4138u, 0x2037A07Bu, 0xC61B63D5u, 0xB1C4301Eu, 0xC3E91EA9u, 0x17596D1u, 0x57389503u, 0x544948F6u, 0x9D28D732u, 0x9A1E1AD0u, 0x64F67D45u, 0x2535FF1u, 0xB5D1E58Eu, 0xACF68E77u, 0x2468344Au, 0xEAC271DFu, 0x244CB1CCu, 0x8E38F108u, 0x1A124D9Au, 0xEE43C2A6u, 0xE4B4A265u, 0xB57E78DAu, 0x9A2FE175u, 0x5C7B31D5u, 0xCBE908BAu, 0xEB99B7D8u, 0x71DC5023u, 0x2882F59Au, 0x12EA8DE0u, 0xD823214u, 0xAE7F62C6u, 0x10948886u, 0x2413011Fu, 0xAAFC7891u, 0x7AF0FB95u, 0xD0E477F4u, 0xAF14A2F0u, 0x68EF8C85u, 0xB03C8011u, 0x8D25CF52u, 0x30028907u, 0xAA292BF5u, 0xAF409ADDu, 0x21A38B05u, 0xCF6DFB5Eu, 0x5A64E1F3u, 0x5912A0FBu, 0x673A6E4Eu, 0x4BF3564Cu, 0x388AAF9Cu, 0x2B86156Bu, 0x39663909u, 0x1F24415Au, 0x7B3DC9ABu, 0xC31CC250u, 0xEE4430ACu, 0xC97C2600u, 0x8B3D9A2Eu, 0x7B3A4931u, 0x6A6360B4u, 0xFDD1290Au, 0xB3295BEDu, 0x4897E010u, 0xFE3852Bu, 0x13EC054Cu, 0x4D9FA67Bu, 0xE03CBABu, 0xE39C43A6u, 0x1DF5F3CEu, 0xF0655CC8u, 0xC0AFF39Cu, 0x14CA6D1Du, 0x246447EAu, 0xBDEBA207u, 0xF4DE766Cu, 0xF8B1CD86u, 0x8675FC4Au, 0xFF99A201u, 0xC2D16D13u, 0x3A6C98EAu,},
		{0xBAD651A3u, 0xEE956A08u, 0x51B6E376u, 0xFEADC4AFu, 0xF313FC2Au, 0xE1386128u, 0x69973EF7u, 0x54ADA437u, 0x472B0B85u, 0xE1DB4C45u, 0x25472383u, 0x27FD8F8Au, 0xF8705FA1u, 0x79470880u, 0xCA07E96Bu, 0x4377A019u, 0xB4B23CF8u, 0x34BD26Eu, 0xB1A8F8E9u, 0xE2EBC8D3u, 0xDFF5BDCDu, 0x59C45859u, 0x4DCE8BC9u, 0x682E5A80u, 0x6EA171AFu, 0xDA905245u, 0x9958BE46u, 0x80D9A02Fu, 0x4A0F9204u, 0xC048CB3Eu, 0x24AD0A4Fu, 0x20435EB6u, 0x6B2F7F2Du, 0x98DB869Cu, 0xCE233B8Fu, 0x6B3EAB66u, 0x798923CBu, 0x1F0E5D88u, 0xE1C2BECCu, 0x2943124Bu, 0xDEED5CB1u, 0xCE090AF7u, 0xACE23CB8u, 0xAFE50739u, 0x66480B0u, 0x6A0F8160u, 0x91ACA3D5u, 0x73EFDBFEu, 0xC7628510u, 0xE716FF12u, 0x8DE38818u, 0x625F7D79u, 0x8393D4F3u, 0xED5C5223u, 0x54A4BE34u, 0x34078E67u, 0x7598FF0u, 0x4413A3F9u, 0xD6EE2832u, 0x7267067Au, 0x44359858u, 0x8ED919BDu, 0x16809C1Bu, 0xACB5501Fu, 0x670F482Fu, 0x52E82114u, 0x246CB300u, 0x7512EE09u, 0x7D950586u, 0x69B265ADu, 0xE788E3A4u, 0xAA4D566u, 0xA1304581u, 0x65A50526u, 0x7E4EA9D0u, 0x734AD5CCu, 0x8AEA473Cu, 0x10B12BD0u, 0x3EECF102u, 0x3F5635CDu, 0x4ED043D7u, 0x20DECFADu, 0x1E725FCAu, 0x44AB1C29u, 0x7DDA3D7Bu, 0xF293036Du, 0x3B4552D0u, 0x36FC74DEu, 0x1B09C471u, 0xC5B74B44u, 0x4A4B33D6u, 0xA258E547u, 0x45449EEu, 0xF0AEE522u, 0x9343EBACu, 0xFC091802u, 0xEDD6F768u, 0x20016891u, 0x3F815039u, 0xEEF4A756u, 0x4CF29AE4u, 0x9EB9AF90u, 0xBA04B7BDu, 0x4F51CC05u, 0x7EC1EFE7u, 0x944B8498u, 0xA86B6E94u, 0xE1082B02u, 0x70409DADu, 0xDCDFCB31u, 0x62BE650Cu, 0xCE79F69Du, 0xA4B13836u, 0x9C8E1905u, 0xA86A269Fu, 0xF8B562BBu, 0xFD25E8Cu, 0x2F7DD5E6u, 0x8A130723u, 0xD95C09B5u, 0xFDF41639u, 0x4EA359B6u, 0xAEE17A82u, 0xFA243DA8u, 0x33D5F8F3u, 0xE23CE57Fu, 0xE630421u, 0xCBDFFAA8u,},
		{0x4F873F13u, 0xAB0C5133u, 0xE5F23DAu, 0xE3EDBCECu, 0x72E09B35u, 0x96EFD2A3u, 0x50898367u, 0x67F5E36Eu, 0x98DBEBB1u, 0x6EB98E7Bu, 0xB9E6693Cu, 0x4E983F33u, 0xA4EF07EDu, 0xB03BE8CCu, 0xCEF18332u, 0x5E3FFA2u, 0x17F9664Cu, 0xA93BF5B1u, 0x94EF7586u, 0xD121E522u, 0x6E6AAEE3u, 0x62B9AA9Fu, 0x4694BAD6u, 0xCDA65CE3u, 0x8FA56310u, 0x3BF47863u, 0x7F7D9A75u, 0x29F05BEBu, 0x82BB4FA3u, 0x6160DCE0u, 0x399A43F4u, 0x2303D4F7u, 0x6E995171u, 0xF7FE2BF5u, 0xA86A91C9u, 0x6CC000u, 0x45665577u, 0xAF23C04Fu, 0x7D6E7AFBu, 0x91214C8Eu, 0x15E42C94u, 0x59C03DD1u, 0xF43CCF1u, 0x46AC789Cu, 0xC7C0A125u, 0xE1FC951Eu, 0x4881AC9u, 0xF2067E45u, 0xC339B390u, 0xCE78D336u, 0x8BCF2C27u, 0x627E8D24u, 0x428A7679u, 0x214DC919u, 0x55C3D5Bu, 0xE336E7C1u, 0x85401B19u, 0x827A2783u, 0x176F27E2u, 0xC2D5807u, 0x1C46D8AAu, 0x71A6F5FAu, 0x2A793198u, 0x1C446F5Au, 0xFDE8C32Au, 0xD8193DFCu, 0x8639CEE4u, 0xF80BDC1Bu, 0xB19A6E50u, 0x24B7DDEDu, 0xD32B275Au, 0x10DF20D1u, 0x6606CEFCu, 0x6FE1F3C7u, 0xCCB0E33u, 0x4D8A09F2u, 0x383D1E6Bu, 0xEFBC109Bu, 0xB44715FBu, 0xCB0B806Fu, 0x238FC0DDu, 0x4FF3323Cu, 0x99560591u, 0x44A72EA0u, 0x8667A4E2u, 0x74631EB1u, 0x96583D10u, 0xACBF1612u, 0x38C5419Eu, 0x7B75B420u, 0x84A203A1u, 0xF8B6E847u, 0x8C019BC0u, 0x381365Bu, 0xC3918D5Au, 0x67932FBu, 0x4EC12677u, 0x7BA0E614u, 0x18211955u, 0x48F58CD1u, 0x75D804E7u, 0x1CA7E926u, 0xCA671CBCu, 0x1E5E6574u, 0x50E29A1Fu, 0x9C040314u, 0xE30FA590u, 0x2F7233E3u, 0xCE3D5118u, 0xB4AF1F90u, 0xC1B6B62Cu, 0x38AAF3FAu, 0xD5BA1870u, 0x1B227C9u, 0xBBB2FDB9u, 0x69D98E6u, 0xEA74C3BFu, 0x11D5B522u, 0x6FC9ACBAu, 0x17E4F910u, 0xEC740979u, 0x8BF0163Eu, 0xC9299BBu, 0x493951D4u, 0x6036412Cu, 0xB99E71ADu, 0x85FB2087u, 0x5CB05B75u,},
		{0x4A1DF7C2u, 0xC2B1DCC8u, 0xFBABCABDu, 0xBC3CF613u, 0xCDFB9E90u, 0x30ADCC0Eu, 0x14058089u, 0xD0037E5Eu, 0x3C91F56Du, 0x349676F2u, 0xB732571Bu, 0x33717630u, 0x6B23142u, 0xA1E8BD63u, 0x8A4AA712u, 0xBBC941F9u, 0x612203CFu, 0xE95B50ECu, 0x45C0524u, 0x8E80FE9u, 0x4E452E7Bu, 0x5AA80F88u, 0xB83E3A7Cu, 0xCB14E116u, 0x564CC393u, 0x51BB2829u, 0x7725C502u, 0xFB64E638u, 0x3C8933AEu, 0xE27BBBC0u, 0xEAF24FBBu, 0xFED29612u, 0x439574ADu, 0x4E89E60u, 0xF6F67810u, 0x83B24FECu, 0xC7499E91u, 0xF4163144u, 0xB89EDFB5u, 0xA90337EBu, 0x62499AD2u, 0xDF43D864u, 0xBDB82CC8u, 0xC8D6C221u, 0x60CFB5ABu, 0x3677A2B4u, 0x260E3D86u, 0x93C6EC2Du, 0x2F58604Bu, 0x14226C1u, 0xFBA65C6Au, 0xD8EBF5Fu, 0x6C9A4C48u, 0x64CB689Cu, 0xABD359F3u, 0x641CA688u, 0x7B07E5C8u, 0x6E889B4Eu, 0xA50D9BE7u, 0xF1F7456Du, 0x3E904776u, 0x695F84EAu, 0x71B6C611u, 0x9FA13918u, 0xC0D32CFCu, 0x8681A352u, 0x8EB58B12u, 0x6E13FD03u, 0x97AD5CA9u, 0xB62F3540u, 0x3F541651u, 0x5194455u, 0x53B9C804u, 0xEB8F2585u, 0x158763A6u, 0xE651DB1Du, 0x2241EB6Fu, 0xB684A714u, 0xE637EE5Au, 0x685F262Eu, 0xD6812152u, 0xFA105697u, 0x66CD7303u, 0x19064B2Eu, 0x7976C0BFu, 0xBEDF1764u, 0x6DE96C3Cu, 0x35294A27u, 0xB994A37Bu, 0x3A8F931Cu, 0x6D46C4Cu, 0x41C06CE3u, 0xF1768552u, 0xD9BA2B1Cu, 0x4C6089C6u, 0x24ACB369u, 0xA4549AF6u, 0x2FE2CAD1u, 0xDDA71F4Eu, 0xF1EB6277u, 0x473FC582u, 0xE0568CD1u, 0xC30B03C5u, 0x1030469Bu, 0xAC1DB9D3u, 0xB7344CE0u, 0x2B46CB4Eu, 0x8A06574Cu, 0x3739B4D0u, 0x587EAE2Bu, 0x769AB883u, 0x5EAF043Fu, 0x55C09C1Du, 0x690CD662u, 0x49FA81ACu, 0x3D7E4258u, 0x272E3E32u, 0x93F0CC9Au, 0xF24B0E55u, 0xCC6A97E9u, 0x88EC0ADCu, 0x5062E161u, 0x291AC159u, 0x13C3873Bu, 0xCA640D39u, 0x32B64593u, 0x40DCFB23u, 0x7415BB1u,},
		{0x943516DFu, 0x9B02AC18u, 0xCBCEE386u, 0x145BC673u, 0xB996A23Du, 0x4AE6F638u, 0x792048A1u, 0xCE6E2610u, 0xB7912645u, 0xC5DE71B5u, 0x32B77BDCu, 0xC8D26B07u, 0x484B0102u, 0xDFEA9E4u, 0x578A32CBu, 0x7388B7Du, 0x32C1D83Du, 0x6810CB7Du, 0x481610F5u, 0x34A0C948u, 0xA661BC12u, 0x2269B497u, 0x2609FB0u, 0xF9DB6F4u, 0x6AC51B29u, 0xC4C27195u, 0x61593B8Du, 0xDFAC7E20u, 0x66F92BE3u, 0x74649D86u, 0x46CF03A9u, 0xAA720D3Cu, 0xB9690D9Cu, 0x8E35C3BCu, 0x1368FBECu, 0x6C6EB33Fu, 0x7CEE6F4Cu, 0x903C30D7u, 0x758828E9u, 0x6679956Bu, 0xA0499C99u, 0x93E595FFu, 0xB5875BAEu, 0x9EAD939Au, 0xC0F9012Fu, 0xE49E20D4u, 0x425CB32Eu, 0x8B573F67u, 0x55397735u, 0x8A913094u, 0x68B3661Fu, 0xCA5B846Bu, 0xCA578EC2u, 0x26E31607u, 0xFD3B4594u, 0xDAF417CCu, 0x43AC6CDFu, 0x224FB049u, 0x7F3CFC21u, 0x11A4528Eu, 0xC20CB8DAu, 0x351EE12Au, 0x7D7CC28Du, 0x19919D72u, 0x732B6EE8u, 0x316340F8u, 0xBB4759F0u, 0x54DC0E5Cu, 0xA682F6EAu, 0x8644CDFCu, 0x2CF5AFF6u, 0xEB751FC9u, 0x74E3C158u, 0xBAB6768u, 0x876963D3u, 0xF7D80887u, 0xB2821B6Du, 0xF0F36C55u, 0x7AD4574Bu, 0x85B51E04u, 0x353C641Au, 0xE8B31B91u, 0x82C5E479u, 0x211D1768u, 0x1A0D6BC6u, 0x339E346Eu, 0xB4A722Fu, 0xA7175FF1u, 0x3012A1A1u, 0xC561929Eu, 0x77CD2BDAu, 0xC0F0BE57u, 0xBC838D7u, 0x19638E02u, 0x96F18971u, 0xF1B3004Bu, 0xF5D87CDFu, 0xF3FB8882u, 0x6D799DAAu, 0xFC2FDDB0u, 0xCE03154Fu, 0xC92B5945u, 0xE329083u, 0xD37FDFF2u, 0xCC8C1565u, 0x224D70FBu, 0x5699DE66u, 0xF7B6AAADu, 0x66E5D636u, 0xE92DEC42u, 0x6B300F14u, 0xF22155E3u, 0x55DD1F90u, 0xF754BB9Du, 0xAD9BEDBBu, 0x58A1CE39u, 0x88F053F5u, 0x4CB0E448u, 0x36E9346Cu, 0x2038343Eu, 0xCE8C23CEu, 0xCC1071Eu, 0xE54B7EEDu, 0x1DD4CAF0u, 0x9346958Bu, 0xEADB432u, 0x3E71D3Du, 0x6C0F2BFFu,},
		{0x7364E26Du, 0xD6C8A8Cu, 0x621E0B85u, 0xEE30112Au, 0xC6B596A2u, 0xC85AA123u, 0xFADD6AAu, 0xF9AC8151u, 0x8BD4097Fu, 0x6906B01Bu, 0x451FF394u, 0xE8641D72u, 0xF46DE0DAu, 0x78067F5Eu, 0x47FB6CABu, 0x14235BF0u, 0xB73CBF64u, 0x25E87C2Du, 0x8ABA72FCu, 0x8B86146Du, 0xF2023621u, 0x674086ABu, 0x97A901DCu, 0xBAF0A53Du, 0x19DD663Eu, 0x3285DBF3u, 0x1F65F76Eu, 0x88A5B982u, 0xCDFA7D44u, 0x707FF251u, 0x13D228F7u, 0xFCD4578Du, 0xE5B52577u, 0x1E436CEu, 0x28732B80u, 0x59DF6E9u, 0x80E5CF0Au, 0x63E2366Bu, 0xF9811A12u, 0x54C79961u, 0x678F88AFu, 0x44759386u, 0xC5D80C19u, 0x757579B6u, 0x3434FF0Bu, 0x811887E4u, 0x78DD8636u, 0xD20491E6u, 0xDD16FA08u, 0x9E888366u, 0x1168529Fu, 0x8FEF6DF7u, 0x42C1EA9Cu, 0xBFAD7B75u, 0x32CC6FB4u, 0x62D3A98Bu, 0x7B78F88Du, 0xB0BFB774u, 0x39C50D25u, 0x728770DFu, 0x6633504Fu, 0x692AEF59u, 0xA9E33962u, 0x658747D1u, 0x99CDFCDBu, 0x27F15CBu, 0x2E340850u, 0xB3259571u, 0xBF6BB4AEu, 0x8D18DC6Au, 0xA6093E9Cu, 0x7BA77005u, 0x8974A20u, 0x91AEC9B3u, 0xE3652C27u, 0x7A859D91u, 0x3A8373FCu, 0x995AA27Bu, 0x7606D72Cu, 0x92CB0AD7u, 0x27069784u, 0x9DF9CEEFu, 0x4DE5DC36u, 0x7F257B4Du, 0xF0F55C88u, 0xE7883E8Fu, 0x2CF7E28Bu, 0xF42947FAu, 0x3AB87EB5u, 0x911E9B24u, 0x45A43FCEu, 0x8711B5DEu, 0x661B5583u, 0x76CC5BB1u, 0x52C9033Cu, 0x22647B86u, 0xBDC2B25Du, 0xC831C89Cu, 0x71D29A3Eu, 0x56081418u, 0x95E2C6A2u, 0x72CCC5C5u, 0xD7E03161u, 0xCD9681F6u, 0x46EF4341u, 0x3A7779BBu, 0xC767CE16u, 0xE716A1FDu, 0x8BE193BBu, 0xE992D254u, 0x1CCC7905u, 0x3EDA7192u, 0x61DBE496u, 0xD836FF1u, 0x2F1BA884u, 0xCDC03F6Au, 0x5EB9EC5Eu, 0x1A32BBD4u, 0xD59F3CABu, 0x4755B3D9u, 0xFC57DE38u, 0x6627476Cu, 0xBBCC1378u, 0x99B19411u, 0xE812BA0Eu, 0x9BE6B571u, 0x2466903Au, 0x1D06A5AEu,},
	};
}

namespace Bench {
	using namespace Engine::Types;

	using Clock = std::chrono::high_resolution_clock; static_assert(Clock::is_steady);
	using Duration = Clock::duration;
	using TimePoint = Clock::time_point;

	class Context;

	/**
	 * Memory clobber.
	 */
	ENGINE_INLINE inline void clobber() noexcept {
		// Cost: none
		// TODO: should we be using acq_rel or seq_cst here?
		std::atomic_signal_fence(std::memory_order_acq_rel);
	}

	/**
	 * Observes a value as a side effect for the purposes of optimization.
	 * From now on every clobber() will act as a read from @p o for
	 * the purposes of optimization.
	 * 
	 * @param o The expression to observe.
	 */
	template<class T>
	ENGINE_INLINE void observe(T&& o) noexcept {
		#ifdef _MSC_VER
			// Costs: lea, mov
			[[maybe_unused]] const void* volatile unused = &o;
			clobber();
		#else
			// Costs: lea
			asm volatile (""::"g"(&o):"memory");

			// Cost: none
			// This has no cost but has a slightly different effect of only observing only
			// the current value but not future values when clobbered.
			// There is no equivalent or workaround on MSVC that i am aware of.
			//asm volatile ("":"+r"(o));
		#endif
	}

	class SystemInfo {
		public:
			std::string cpu;
			std::string os;
	};

	SystemInfo getSystemInfo();

	class BenchmarkId {
		public:
			std::string name;
			std::string dataset;

			friend decltype(auto) operator<<(std::ostream& os, const BenchmarkId& id) {
				return os << id.name << "/" << id.dataset;
			}

			ENGINE_INLINE bool operator==(const BenchmarkId& rhs) const noexcept {
				return name == rhs.name && dataset == rhs.dataset;
			}
	};
}

template<>
struct fmt::formatter<Bench::BenchmarkId> : fmt::formatter<std::string_view> {
	auto format(const Bench::BenchmarkId& id, auto& ctx) {
		return fmt::format_to(ctx.out(), "{}/{}", id.name, id.dataset);
	}
};

template<>
struct Engine::Hash<Bench::BenchmarkId> {
	[[nodiscard]] ENGINE_INLINE size_t operator()(const Bench::BenchmarkId& val) const {
		auto seed = Engine::hash(val.name);
		Engine::hashCombine(seed, Engine::hash(val.dataset));
		return seed;
	}
};

namespace Bench {
	class Benchmark {
		public:
			using Func = void(*)();

		public:
			Func iterFunc;
			Func singleFunc;
			int64 size;
	};
	
	class Group {
		private:
			Engine::FlatHashMap<BenchmarkId, Benchmark> benchmarks;
			uint64 warmups = 10;
			uint64 iters = 100;

		public:
			const auto& getBenchmarks() const { return benchmarks; }

			void setup(uint64 warmups, uint64 iters) {
				this->warmups = warmups;
				this->iters = iters;
			}

			int add(BenchmarkId id, Benchmark bench) {
				const auto found = benchmarks.find(id);
				if (found != benchmarks.end()) {
					ENGINE_WARN("Benchmark \"", id, "\" already exists.");
					id.name += "~DUPLICATE~";
					return add(id, bench);
				} else {
					benchmarks.emplace(id, bench);
				}
				return 0;
			}

	};
	
	enum class StatInterp {
		Calc,
		Flat,
	};

	class StoredValueBase {
		public:
			StoredValueBase() = default;
			StoredValueBase(const StoredValueBase&) = delete;

			virtual std::string str() const = 0;
			virtual const void* get() const = 0;
	};

	template<class T>
	class StoredValue : public StoredValueBase {
		private:
			static_assert(std::same_as<T, std::remove_cvref_t<T>>);
			T value;

		public:
			template<class U>
			StoredValue(U&& val) : value{std::forward<U>(val)} {
			};
			
			virtual std::string str() const override {
				return fmt::format("{}", value);
			}

			virtual const void* get() const override {
				return &value;
			}
	};

	template<class T>
	class SampleProperties {
		public:
			T min = {};
			T max = {};
			T mean = {};
			T stddev = {};

		public:
			SampleProperties() = default;

			template<class U>
			SampleProperties(SampleProperties<U> other)
				: min(other.min)
				, max(other.max)
				, mean(other.mean)
				, stddev(other.stddev) {
			}

			SampleProperties scaleN(long double x) {
				min /= x;
				max /= x;
				mean /= x;
				stddev /= std::sqrt(x);
				return *this;
			}
	};

	template<class Range>
	auto calcSampleProperties(const Range& input) {
		using T = std::remove_cvref_t<decltype(*std::ranges::begin(input))>;
		SampleProperties<T> props = {};
		if (std::ranges::empty(input)) { return props; }

		props.min = *std::ranges::begin(input);
		props.max = props.min;
		for (T c = {}; const auto val : input) {
			props.min = std::min(props.min, val);
			props.max = std::max(props.max, val);

			// Kahan Summation
			const auto y = val - c;
			const auto t = props.mean + y;
			c = (t - props.mean) - y;
			props.mean = t;
		}

		const auto size = std::ranges::size(input);
		props.mean /= size;

		for (T c = {}; const auto& val : input) {
			const auto diff = val - props.mean;

			// Kahan Summation
			const auto y = diff * diff - c;
			const auto t = props.stddev + y;
			c = (t - props.stddev) - y;
			props.stddev = y;
		}

		props.stddev = static_cast<T>(std::sqrt(props.stddev / input.size()));

		return props;
	}

	class Context {
		private:
			Engine::FlatHashMap<std::string, Group> groups;
			Engine::FlatHashMap<std::string, std::unique_ptr<StoredValueBase>> custom;
			TimePoint sampleStart;
			TimePoint sampleStop;
			std::vector<Duration> samples;

		public:
			Context() {};

			nullptr_t setup(const std::string& name, uint64 warmups, uint64 iters) { groups[name].setup(warmups, iters); return nullptr; }
			ENGINE_INLINE bool hasGroup(const std::string& name) { return groups.contains(name); }
			ENGINE_INLINE auto& getGroup(const std::string& name) { return groups[name]; }

			/*
			 * We usually want to sample the whole dataset at once
			 * because the cost of `Clock::now()` could actually out weigh our operation we are benchmarking.
			 * For example, on MSVC `now` performs: 2x call, 4x idiv, 2x imul, 8x other
			 * whereas just doing there vector iteration is just an: add, cmp, jne
			 * So timing the loop as a whole should give us more accurate numbers.
			 */
			ENGINE_INLINE void startSample() noexcept {
				std::atomic_thread_fence(std::memory_order_acq_rel);
				sampleStart = Clock::now();
				std::atomic_thread_fence(std::memory_order_acq_rel);
			}
			
			ENGINE_INLINE void stopSample() noexcept {
				std::atomic_thread_fence(std::memory_order_acq_rel);
				sampleStop = Clock::now();
				std::atomic_thread_fence(std::memory_order_acq_rel);
				samples.push_back(sampleStop - sampleStart);
			}

			ENGINE_INLINE static Context& instance() { static Context inst; return inst; }

			template<class T>
			ENGINE_INLINE static auto& getDataset() { static T inst; return inst; }

			void runGroup(const std::string& name);

			// TODO: should also take optional format string
			template<class T>
			void set(const std::string& col, T&& value, StatInterp = {}) {
				// TODO: different interp modes - should this be only the value set or computed over all iterations (like we do for default avg, etc.)
				custom[col] = std::make_unique<StoredValue<std::remove_cvref_t<T>>>(value);
			}
	};
}

// TODO: a lot of this could probably be constexpr/consteval-ified

#define BENCH_CONCAT_IMPL(a, b) a##b
#define BENCH_CONCAT(a, b) BENCH_CONCAT_IMPL(a, b)

#define BENCH_DEFINE_COMPILE_TYPE_DEF_CHECK(Name, Negate, Message)\
	template<class T, unsigned long long line, class = void> struct _bench_compile_check_##Name { static_assert(Negate || (line != line), Message); };\
	template<class T, unsigned long long line> struct _bench_compile_check_##Name<T, line, std::void_t<decltype(sizeof(T))>> { static_assert(!Negate || (line != line), Message); };

#define BENCH_USE_COMPILE_CHECK(Name, Type)\
	namespace { struct Type; }\
	static _bench_compile_check_##Name<Type, __LINE__> BENCH_CONCAT(_bench_compile_check_##Name##_var, __LINE__);

/** Check for BENCH_GROUP */
BENCH_DEFINE_COMPILE_TYPE_DEF_CHECK(single_group_per_unit, true, "You many only have one benchmark group per translation unit.");

/**
 * Defines a new benchmark group.
 */
#define BENCH_GROUP(Name, Warmups, Iters, ...)\
	BENCH_USE_COMPILE_CHECK(single_group_per_unit, _bench_group_id)\
	namespace { struct _bench_group_id { \
		constexpr static char name[] = Name;\
		static inline nullptr_t dumby = Bench::Context::instance().setup(Name, Warmups, Iters); \
	};}
	

/**
 * Defines a benchmark function.
 * A benchmark function is given:
 * - The context it is running in - param `ctx`
 * - The dataset it should operate on - param `dataset`, type of template argument `D`
 * - A constexpr bool that is set only on the first pass before benchmark warming - `SinglePass`
 */
#define BENCH(Name)\
	template<class D, bool SinglePass> void Name(Bench::Context& ctx = Bench::Context::instance(), const D& dataset = Bench::Context::getDataset<D>())

/**
 * Defines a benchmark with a specific group, function, and dataset.
 * @see BENCH_USE
 */
#define BENCH_USE_GROUP(Group, Name, Dataset)\
	static auto BENCH_CONCAT(_bench_##Name##_var_, __LINE__) = Bench::Context::instance().getGroup(Group).add(\
		{#Name, #Dataset},\
		{[]{ Name<Dataset, false>(); }, []{ Name<Dataset, true>(); }, Bench::Context::getDataset<Dataset>().size() }\
	);

/** Check for BENCH_USE */
BENCH_DEFINE_COMPILE_TYPE_DEF_CHECK(use_group_defined, false, "You must define a group before using BENCH_USE or specify a group with BENCH_USE_GROUP.");

/**
 * Defines a benchmark with a specific function/dataset pair.
 * Is assigned to the group specified by BENCH_GROUP.
 * @see BENCH_USE_GROUP
 */
#define BENCH_USE(Name, Dataset)\
	BENCH_USE_COMPILE_CHECK(use_group_defined, _bench_group_id)\
	BENCH_USE_GROUP(_bench_group_id::name, Name, Dataset)

