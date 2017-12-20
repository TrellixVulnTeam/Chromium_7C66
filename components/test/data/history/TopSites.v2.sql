-- unit_tests --gtest_filter=TopSitesDatabaseTest.Version2
--
-- .dump of a version 2 "Top Sites" database.  The Google thumbnail
-- matches kGoogleThumbnail from thumbnail-inl.h.
BEGIN TRANSACTION;
CREATE TABLE meta(key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY, value LONGVARCHAR);
INSERT INTO "meta" VALUES('version','2');
INSERT INTO "meta" VALUES('last_compatible_version','2');
CREATE TABLE thumbnails (url LONGVARCHAR PRIMARY KEY,url_rank INTEGER ,title LONGVARCHAR,thumbnail BLOB,redirects LONGVARCHAR,boring_score DOUBLE DEFAULT 1.0, good_clipping INTEGER DEFAULT 0, at_top INTEGER DEFAULT 0, last_updated INTEGER DEFAULT 0, load_completed INTEGER DEFAULT 0);
INSERT INTO "thumbnails" VALUES('http://www.google.com/chrome/intl/en/welcome.html',1,'Welcome to Chromium',NULL,'http://www.google.com/chrome/intl/en/welcome.html',1.0,0,0,13022203361871994,0);
INSERT INTO "thumbnails" VALUES('https://chrome.google.com/webstore?hl=en',2,'Chrome Web Store',NULL,'https://chrome.google.com/webstore?hl=en',1.0,0,0,13022203361875499,0);
INSERT INTO "thumbnails" VALUES('http://www.google.com/',0,'Google',X'ffd8ffe000104a46494600010100000100010000ffdb0043000302020302020303030304030304050805050404050a070706080c0a0c0c0b0a0b0b0d0e12100d0e110e0b0b1016101113141515150c0f171816141812141514ffdb00430103040405040509050509140d0b0d1414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414ffc0001108008800c403012200021101031101ffc4001f0000010501010101010100000000000000000102030405060708090a0bffc400b5100002010303020403050504040000017d01020300041105122131410613516107227114328191a1082342b1c11552d1f02433627282090a161718191a25262728292a3435363738393a434445464748494a535455565758595a636465666768696a737475767778797a838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae1e2e3e4e5e6e7e8e9eaf1f2f3f4f5f6f7f8f9faffc4001f0100030101010101010101010000000000000102030405060708090a0bffc400b51100020102040403040705040400010277000102031104052131061241510761711322328108144291a1b1c109233352f0156272d10a162434e125f11718191a262728292a35363738393a434445464748494a535455565758595a636465666768696a737475767778797a82838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae2e3e4e5e6e7e8e9eaf2f3f4f5f6f7f8f9faffda000c03010002110311003f00fd269bc157f27899f564f146b3140d324bfd98b2c46d805099400c65829dac4fcd9f9cf200188750f06ebd75a8493dbf8baf6ce1791dc5bac71b2a827214123381d3e9f9d767456d4eaca93bc6df349fe626ae72fadf86356d527824b5f115de98238d119615560e54e4b1046327a1e318a6eb9e15d6354b9825b4f12de69612211bc702a32c878f98ee5383c76f535d55156b1138dad6d3c97f90ac725ad784759d46fd6e6d3c4f7ba6a845468225568d88182704704f5e2a4d6bc2fabea57265b6f125de9c86158bcb855186e19cbf2bd4e7e9c74aea68a1622a2b6da792ff0020e54735aa786f56beb88e4b7f10dd58a2c6a8638d518311d5b91d4d6dd95b4b67018da57b83bddf7ccf96f998b6de00e06703d80ebd6ad5159caa4a4945f4f243b586664feeaffdf5ff00d6a3327f757fefaffeb53e8acc633327f757fefaff00eb51993fbabff7d7ff005a9f450033327f757fefaffeb51993fbabff007d7ff5ab90f8bbf11a0f853f0ff54f124d01bb7b6091c16c1b6f9b33b848d73d8166193e99ac9bcf0b7c44b8f0fc33da78e23b6d770b23c1269b03d913c164c6df331d467766b78d26e2a726926edadfa6fb5fba3373d5c56ad1e8b993fbabff007d7ff5a8cc9fdd5ffbebff00ad5c3789be33f877c24f7e2f5aeae2df4b648f54bdb1b7334160cc01fdf15e570086380768209c66b5f53f885a3e992c885e6bb586249e796ce2332431b8cab315ec473c678e7a115e7d4c4d1a49b9c92b7fc1ff0027f73ec763c3564949c5d9edf87f9afbd773a2cc9fdd5ffbebff00ad4664feeaff00df5ffd6af3ff0019fc77f08f81ec65bcbcbc96f2d20485e7b8b08fce8a1597fd5191c1da81b83924000824804679ff000c7c5596cbc63e3e8b59bdb8bbb4b7d52d2cb49d3d624f3b74968b3346a06327976258f014f3c574d4bd28c2725a4b67f2bff97de8e35352a8a94756ff00c9bfd19ec1993fbabff7d7ff005a8cc9fdd5ff00bebffad5ce781be20e97f10f4e37da38b87b65792195a68f618a646daf13027218119f4208209cd74d52a4a4aeb6379c254e4e1356686664feeaff00df5ffd6a3327f757fefaff00eb53e8aa206664feeaff00df5ffd6ae7fc4be19d475e9a27b5f116a5a1840014b030957c1279f32263dc7423a0f7cf47452038993c07ad3b964f1c6bb1038f9545a91d31fc56e68aeda8a7715828a28a430a28a2800a28a2800a28a2800a28a2800a2a87fc241a5e587f695a6549523cf5e083823af5068fedfd2ffe82569ff7fd7fc6a799771d99cd7c61f86365f183e1e6ade15bdb892cd6f155a2ba8865e095183c6e077c328c8ee3238cd709ae695f17bc43e03baf0f4f6d61a76bc96cd0db789748d6a4851e5da5566687cbdc3fbc537119ef5ec1fdbfa5ff00d04ad3feff00aff8d1fdbfa5ff00d04ad3feff00aff8d75d3c5ca9c542e9a4eeafd1e9f9d95d6c632a2a4dcbbe878c697f06bc53e17b5f1ee95637765aa69de33dd713dcdfbb799657125bac12e571fbd421030e54f639eb5d1e83f0e359f014379a76886db51d32f34fb6b406f242af04914021de460ef5650a7190462bd13fb7f4bffa095a7fdff5ff001a3fb7f4bffa095a7fdff5ff001af1ea616854494ba79f74d3fc1bfe923d3a98baf57e27f87a6beba2fe9b3e39f17782a5d23c2be32b7f0ada5e789bc3fa60834df1adacf751c097ff0066895e636fb8128e10a06e402148da7a9f41d67e06eb9e2eb9d43c4fa3cba632dd6ab67afe9767a8a33453c42cc40f6f70b8f9772313919c1c715eaf7fe04f87ba96bf71ad5c5a696fa9dc10679bcf03ce20601750d863803920f4aeb135cd2a34555d46cd55460013a600fcebd4c54e857a34e8a5650db5f2492dfa5b47a6965b2479f875530f5feb09fbcaff008dd3fbd37a7cf72b78461be874385751d36cf49bb3cbdad8c9e6469ff02dab9fae2b6aa87f6fe97ff412b4ff00bfebfe347f6fe97ff412b4ff00bfebfe358292ee68eeddec5fa2a87f6fe97ff412b4ff00bfebfe34b2ebba6c0c164d46d2362a1c069d412a7a1ebd3deaa3ef3b4752762f5159dff091693c7fc4cecf9e9fe909cfeb57a19a3b8892589d658dc655d0e430f506a9c651dd00fa28a2a4028a28a0028a28a0028a28a0028a28a0028a28a00cdb1d36e34eb7f221b888c61dd87990927e662c790c3d6ac7977bff003f107fdf86ff00e2ead5151c8bfa6cae6663eb9a8dde89a4dcdf3c90cab0aee28b0364f38fefd793cffb4f68f69007ba59ad98bf96237b09492d9c60152437afca4f1cd7b45d5b5bea16cd0ce893c120c946e430073fe159bff086e859c7f65db67fdc152e1d9fe23523cbcfed25a7473491cd05c40cb2796b9d3e57dfe846c270383d7078e9d2af0f8f111d1e3d4c59cc6d9dc2605a3f98bf2b312537670029e80e78c6722bd05bc21a0a0cb69b6aa3dd053c78434431ec1a65b6dceec041d69723ee1cde47974ffb47d8db246d2da5d279a47943fb3e526453fc430781c1e1b078e9d29a9fb4ae9334124d1ef92343b7e5b297716cb02bb49ce415e7200191cd7a87fc21fa0e14ff0066dafcdd3e41cd1ff087e843fe6196bd71f7051c8fb87379187e02f8807e202ddc96452386058db74b6ce84962e31b598118d9dc77aeb7cbbdff009f883fefc37ff1751e9da2d868e643656915a99701cc6b82d8ce33f4c9fceaf55282ebf9b173157cbbdff9f883fefc37ff00175e69e3ef046a9aaeb91dc59e8d6ba8a08d10cf2ac24e4673c4849039edfae2bd568aebc3d67869fb486afceffe64cbde56678ee87e08d42c6091af3c0da55f3b3305b7963b558e3008c32900fdee49e3d3f0f5eb7b78ad20486089218506d48e350aaa3d001d2a4a2b4c46267887792b7a5ff56c951b0514515c850514514005145140051451400514514005145140052119183d296992ab3c4ea8fe5b95215f19da7b1c77a00e426f843e13b850afa6c85463817938e01071c3f4c8191d0e39a9d3e1778691b77f67b31dd905ae24f97d87cdd3be2aa43e08f11432961e38bf923c604725ac2707180738c93fa64938e9890f837c43e4e078ceefce0a54486d23c6371232bd0900819e38519ef9ebf634ff00e7eafba5fe44ddf63407c3fd07ed97b746c99e7bc0e2667b895b76fceec02d85ea718c632718a862f869e1e827b7956d25dd6ec1a206ea521581273f7b9ce79ce4600150c5e1ed574a82fae350f185c3db7d9650649a18a2580f5f37776da01ebf5ac183cc9628648fe29c12c687cb2ca2d8ac876f009cfdec60f047d292a54ef6f68bee97f90eefb1beff000b7c3a6d7ecf15a4b6f0e41d915c498e3381824e073d07b7a0a897e117860c16d14d673dcadbb33c666bc9890c58b13f7f9393deab5a687aa6b123dce9fe3d7b8814ec75b68629103617b8270780719fe23c7231a53785b5a9b4a8edc78aef12ed26f33ed6b6f102cbcfc85718239fd07e3a34b9153f6cb97b7bda7e02f3b0d6f85fe1a680446c65281f78ff004c9f21b00641df9e805741a5699068da7c1656a19608576a0772ed8f72793f8d736de12f103468078bee52405b320b48cee076e060e4718f4ee6b6bc35a55f68fa60b7d43569759b80c5bed3344b1b63038c2f1ebf9d613a708c6ea69bedafea877f23568a28ac061451450014514500145145001451450014514500145145001451450014514500145145003648d268d91d43a302acac32083d411559749b158bcb1656e23c83b044b8cfd3156e8a00af67616ba746d1dadb436c8c77158630809c019c0ef803f2ab14514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451400514514005145140051451401fffd9','https://www.google.com/ http://www.google.com/',0.836942538593482,1,1,13022203426208198,1);
COMMIT;
