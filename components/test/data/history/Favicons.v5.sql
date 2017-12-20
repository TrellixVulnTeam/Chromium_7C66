-- unit_tests --gtest_filter=ThumbnailDatabaseTest.Version5
--
-- .dump of a version 5 Favicons database.
BEGIN TRANSACTION;
CREATE TABLE meta(key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY, value LONGVARCHAR);
INSERT INTO "meta" VALUES('version','5');
INSERT INTO "meta" VALUES('last_compatible_version','5');
CREATE TABLE favicons(id INTEGER PRIMARY KEY,url LONGVARCHAR NOT NULL,last_updated INTEGER DEFAULT 0,image_data BLOB,icon_type INTEGER DEFAULT 1,sizes LONGVARCHAR);
INSERT INTO "favicons" VALUES(1,'http://www.google.com/favicon.ico',1287424416,X'313233343631303233353631323033393437353136333435313635393133343837313034373831323336343931363534313932333435313932333435313233343931333400',1,'32 32');
INSERT INTO "favicons" VALUES(2,'http://www.google.com/touch.ico',1287424428,X'676F6977756567727172636F6D697A71797A6B6A616C697462616878666A7974727176707165726F6963786D6E6C6B686C7A756E616378616E65766961777274786379776867656600',2,'32 32');
INSERT INTO "favicons" VALUES(3,'http://www.yahoo.com/favicon.ico',1287424428,X'676F6977756567727172636F6D697A71797A6B6A616C697462616878666A7974727176707165726F6963786D6E6C6B686C7A756E616378616E65766961777274786379776867656600',1,'32 32');
CREATE TABLE icon_mapping(id INTEGER PRIMARY KEY,page_url LONGVARCHAR NOT NULL,icon_id INTEGER);
INSERT INTO "icon_mapping" VALUES(1,'http://google.com/',1);
INSERT INTO "icon_mapping" VALUES(2,'http://www.google.com/',1);
INSERT INTO "icon_mapping" VALUES(3,'http://www.google.com/',2);
INSERT INTO "icon_mapping" VALUES(4,'http://yahoo.com/',3);
INSERT INTO "icon_mapping" VALUES(5,'http://www.yahoo.com/',3);
CREATE INDEX favicons_url ON favicons(url);
CREATE INDEX icon_mapping_page_url_idx ON icon_mapping(page_url);
CREATE INDEX icon_mapping_icon_id_idx ON icon_mapping(icon_id);
COMMIT;
