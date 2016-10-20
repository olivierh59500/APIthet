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

#include "mainwindow.h"
#include "ui_mainwindow.h"

//Handle Replies
void MainWindow::replyFinished (QNetworkReply *reply)
{
    ui->textBrowser->clear();
    ui->textBrowser->append(reply->readAll().constData());
    checkSecHeaders(reply);

    switch(attackType) {
    case XSS:
        processXssReply(reply);
        break;
    case CSRF:
        processCsrfReply(reply);
        break;
    case SQL_INJ:
    case HTML_INJ:
    case OPEN_REDIRECT:
    case HTTP_SPLITTING:
    case CRLF:
    case CORS:
    case HEADER_FUZZ:
    default:
        break;
    }

    //check if an operation was performed without an Auth header
    if (!reply->error() && !headerHasAuth && (hasQueryParams || postQueryHasBody))
            likelyUnauth = true;

    reply->deleteLater();

    //end of the event loop
    eventLoop.quit();
}

void MainWindow::processCsrfReply(QNetworkReply *reply)
{
    if(reply->error())
    {
        ui->textBrowser->append("An error occured while performing the operation...");
        ui->textBrowser->append(reply->errorString());
    }

    else
        csrfIssueLikely = true;
}

void MainWindow::processXssReply(QNetworkReply *reply)
{
    if(reply->error())
    {
        ui->textBrowser->append("An error occured while performing the operation...");
        ui->textBrowser->append(reply->errorString());
    }

    else {
        QByteArray hostResponse = reply->readAll();
        QString replyStr = QString(hostResponse);

        //Analyze the reply, check if it contains any malicious payload
        if (replyStr.contains(maliciousPart1) && replyStr.contains(maliciousPart2))
            analyzePayload(replyStr);

        else if (currentParam.length()) {
            ui->textBrowserResults->append
                    (QString("<font color=red>Stored XSS likely for JSON param -- %1</font>").arg(currentParam));
            ui->textBrowserResults->append
                    ("<i>Malicious payload was injected, but application replied with HTTP status OK</i>");
            ui->textBrowserResults->append
                    ("XSS (OWASP), CWE 79");
            ui->textBrowserResults->append("------------------------------------");
        }
    }
}
