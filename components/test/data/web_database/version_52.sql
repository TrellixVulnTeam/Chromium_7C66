PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE meta(key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY, value LONGVARCHAR);
INSERT INTO "meta" VALUES('version','52');
INSERT INTO "meta" VALUES('last_compatible_version','48');
INSERT INTO "meta" VALUES('Default Search Provider ID','2');
INSERT INTO "meta" VALUES('Builtin Keyword Version','62');
CREATE TABLE web_intents ( service_url LONGVARCHAR, action VARCHAR, type VARCHAR, title LONGVARCHAR, disposition VARCHAR, scheme VARCHAR, UNIQUE (service_url, action, scheme, type));
CREATE TABLE web_intents_defaults ( action VARCHAR, type VARCHAR, url_pattern LONGVARCHAR, user_date INTEGER, suppression INTEGER, service_url LONGVARCHAR, scheme VARCHAR, UNIQUE (action, scheme, type, url_pattern));
CREATE TABLE keywords (id INTEGER PRIMARY KEY,short_name VARCHAR NOT NULL,keyword VARCHAR NOT NULL,favicon_url VARCHAR NOT NULL,url VARCHAR NOT NULL,safe_for_autoreplace INTEGER,originating_url VARCHAR,date_created INTEGER DEFAULT 0,usage_count INTEGER DEFAULT 0,input_encodings VARCHAR,show_in_default_list INTEGER,suggest_url VARCHAR,prepopulate_id INTEGER DEFAULT 0,created_by_policy INTEGER DEFAULT 0,instant_url VARCHAR,last_modified INTEGER DEFAULT 0,sync_guid VARCHAR,alternate_urls VARCHAR,search_terms_replacement_key VARCHAR,image_url VARCHAR,search_url_post_params VARCHAR,suggest_url_post_params VARCHAR,instant_url_post_params VARCHAR,image_url_post_params VARCHAR);
INSERT INTO "keywords" VALUES(2,'Google','google.com','http://www.google.com/favicon.ico','{google:baseURL}search?q={searchTerms}&{google:RLZ}{google:originalQueryForSuggestion}{google:assistedQueryStats}{google:searchFieldtrialParameter}{google:searchClient}{google:sourceId}{google:instantExtendedEnabledParameter}{google:omniboxStartMarginParameter}ie={inputEncoding}',1,'',0,0,'UTF-8',1,'{google:baseSuggestURL}search?{google:searchFieldtrialParameter}client={google:suggestClient}&q={searchTerms}&{google:cursorPosition}{google:zeroPrefixUrl}{google:pageClassification}sugkey={google:suggestAPIKeyParameter}',1,0,'{google:baseURL}webhp?sourceid=chrome-instant&{google:RLZ}{google:instantEnabledParameter}{google:instantExtendedEnabledParameter}{google:ntpIsThemedParameter}{google:omniboxStartMarginParameter}ie={inputEncoding}',0,'2BB4ECA1-7D0F-2FB9-45B0-AD87B8298C9D','["{google:baseURL}#q={searchTerms}","{google:baseURL}search#q={searchTerms}","{google:baseURL}webhp#q={searchTerms}"]','espv','{google:baseURL}searchbyimage/upload','','','','encoded_image={google:imageThumbnail},image_url={google:imageURL},sbisrc={google:imageSearchSource}');
INSERT INTO "keywords" VALUES(3,'Bing','bing.com','http://www.bing.com/s/wlflag.ico','http://www.bing.com/search?setmkt=en-US&q={searchTerms}',1,'',0,0,'UTF-8',1,'http://api.bing.com/osjson.aspx?query={searchTerms}&language={language}',3,0,'',0,'A0226B1A-8662-A160-C4D4-078608DBD17E','[]','','','','','','');
INSERT INTO "keywords" VALUES(4,'Yahoo!','yahoo.com','http://search.yahoo.com/favicon.ico','http://search.yahoo.com/search?ei={inputEncoding}&fr=crmas&p={searchTerms}',1,'',0,0,'UTF-8',1,'http://ff.search.yahoo.com/gossip?output=fxjson&command={searchTerms}',2,0,'',0,'531278E0-9380-7556-D9A0-9F85AA1DCF06','[]','','','','','','');
INSERT INTO "keywords" VALUES(5,'AOL','aol.com','http://search.aol.com/favicon.ico','http://search.aol.com/aol/search?query={searchTerms}',1,'',0,0,'UTF-8',1,'http://autocomplete.search.aol.com/autocomplete/get?output=json&it=&q={searchTerms}',35,0,'',0,'680838DC-A89F-7598-901A-C6B849F94F67','[]','','','','','','');
INSERT INTO "keywords" VALUES(6,'Ask','ask.com','http://sp.ask.com/sh/i/a16/favicon/favicon.ico','http://www.ask.com/web?q={searchTerms}',1,'',0,0,'UTF-8',1,'http://ss.ask.com/query?q={searchTerms}&li=ff',4,0,'',0,'25AC092A-10E6-EA19-11CE-55382548F3DC','[]','','','','','','');
CREATE TABLE token_service (service VARCHAR PRIMARY KEY NOT NULL,encrypted_token BLOB);
CREATE TABLE web_app_icons (url LONGVARCHAR,width int,height int,image BLOB, UNIQUE (url, width, height));
CREATE TABLE web_apps (url LONGVARCHAR UNIQUE,has_all_images INTEGER NOT NULL);
CREATE TABLE autofill (name VARCHAR, value VARCHAR, value_lower VARCHAR, pair_id INTEGER PRIMARY KEY, count INTEGER DEFAULT 1);
CREATE TABLE credit_cards ( guid VARCHAR PRIMARY KEY, name_on_card VARCHAR, expiration_month INTEGER, expiration_year INTEGER, card_number_encrypted BLOB, date_modified INTEGER NOT NULL DEFAULT 0, origin VARCHAR DEFAULT '');
CREATE TABLE autofill_dates ( pair_id INTEGER DEFAULT 0, date_created INTEGER DEFAULT 0);
CREATE TABLE autofill_profiles ( guid VARCHAR PRIMARY KEY, company_name VARCHAR, address_line_1 VARCHAR, address_line_2 VARCHAR, city VARCHAR, state VARCHAR, zipcode VARCHAR, country VARCHAR, country_code VARCHAR, date_modified INTEGER NOT NULL DEFAULT 0, origin VARCHAR DEFAULT '');
CREATE TABLE autofill_profile_names ( guid VARCHAR, first_name VARCHAR, middle_name VARCHAR, last_name VARCHAR);
CREATE TABLE autofill_profile_emails ( guid VARCHAR, email VARCHAR);
CREATE TABLE autofill_profile_phones ( guid VARCHAR, type INTEGER DEFAULT 0, number VARCHAR);
CREATE TABLE autofill_profiles_trash ( guid VARCHAR);
CREATE INDEX web_intents_index ON web_intents (action);
CREATE INDEX web_intents_default_index ON web_intents_defaults (action);
CREATE INDEX web_apps_url_index ON web_apps (url);
CREATE INDEX autofill_name ON autofill (name);
CREATE INDEX autofill_name_value_lower ON autofill (name, value_lower);
CREATE INDEX autofill_dates_pair_id ON autofill_dates (pair_id);
COMMIT;
