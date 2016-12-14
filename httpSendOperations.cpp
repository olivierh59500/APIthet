/*
 *  Copyright [2016] [mail.gulpfriction@gmail.com]
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "apithet.h"
#include "ui_apithet.h"

//Set the correct HTTP Method
void APIthet::sendGEThttp()
{
    QNetworkRequest httpRequest;
    QString urlString = ui->lineEditURL->text();
    //QString randomString;
    QUrl targetURL = QUrl(urlString);
    QString strQuery = targetURL.query(QUrl::PrettyDecoded);

    setApplicationProxy();
    //Add headers to the request
    setHttpHeaders(&httpRequest);

    //Check if the URL contains any key value pairs
    if (strQuery.length() && !(strQuery.isEmpty())) {
        genKeyValueMap(&targetURL);

        hasQueryParams = true;

        //Check if the query likely contains password
        if (strQuery.contains("pass") || strQuery.contains("password") ||
                strQuery.contains("paswd") || strQuery.contains("passwd"))
            passwdInGetQuery = true;

        //perform HTML injection attack
        performHtmlInjection(&httpRequest);

        //perform XSS attack
        performUrlXSS(&httpRequest);

        //perform sqli on url parameters
        performUrlQuerySQLI(&httpRequest);

        //perform open redirect attack
        performOpenRedirect(&httpRequest);

        //Clear the query param map
        urlParamMap.clear();
    }

    else {
        //perform in-band url based sql injection
        performUrlSQLI(&httpRequest);
        //Set URL
        httpRequest.setUrl(targetURL);
        //Invoke get method
        manager->get(httpRequest);
        eventLoop.exec();
    }
    //CSRF Request
    performGetCSRF(&httpRequest);
}

void APIthet::sendPOSThttp()
{
    QNetworkRequest httpRequest;
    //;

    QString strQuery = ui->lineEditURL->text();
    QUrl targetURL = QUrl(ui->lineEditURL->text());
    QString postPayload = ui->plainTextEditPayload->toPlainText();
    QByteArray reqPayloadBinary = postPayload.toUtf8();


    QMap<QString, quint16>::iterator jsonParamsEntry;

    //form a json map from the payload
    bool validJsonPayload = genJSONpayload(reqPayloadBinary);

    setApplicationProxy();

    //initialize the iterator to beginning of the map element
    jsonParamsEntry = jsonParamsMap.begin();

    //Set URL
    httpRequest.setUrl(targetURL);

    //Add headers to the request
    setHttpHeaders(&httpRequest);

    if (postPayload.length())
        postQueryHasBody = true;

    //likely contains password
    if (strQuery.contains("pass") || strQuery.contains("password") ||
        strQuery.contains("paswd") || strQuery.contains("passwd"))
            passwdInPostQuery = true;

    if (validJsonPayload)
        performJsonXSS(&httpRequest);


    performPostCSRF(&httpRequest);

    //Invoke post method
    if (!validJsonPayload)
        manager->post(httpRequest, ui->plainTextEditPayload->toPlainText().toUtf8());
}

void APIthet::sendPUThttp()
{
    QNetworkRequest httpRequest;
    QUrl targetURL = QUrl(ui->lineEditURL->text());
    QString strQuery = targetURL.query(QUrl::PrettyDecoded);

    //if (strQuery.length())
    //    ui->textBrowser->append(strQuery);

    //Set URL
    httpRequest.setUrl(targetURL);

    //Add headers to the request
    setHttpHeaders(&httpRequest);

    //Invoke post method
    manager->put(httpRequest, ui->plainTextEditPayload->toPlainText().toUtf8());
    //ui->pushButtonAdd->setEnabled(false);
}
