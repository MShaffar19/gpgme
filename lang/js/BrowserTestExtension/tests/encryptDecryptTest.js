/* gpgme.js - Javascript integration for gpgme
 * Copyright (C) 2018 Bundesamt für Sicherheit in der Informationstechnik
 *
 * This file is part of GPGME.
 *
 * GPGME is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * GPGME is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 */

describe('Encryption and Decryption', function () {
    it('Successful encrypt and decrypt simple string', function (done) {
        let prm = Gpgmejs.init();
        prm.then(function (context) {
            context.encrypt(
                inputvalues.encrypt.good.data,
                inputvalues.encrypt.good.fingerprint).then(function (answer) {
                    expect(answer).to.not.be.empty;
                    expect(answer.data).to.be.a("string");
                    expect(answer.data).to.include('BEGIN PGP MESSAGE');
                    expect(answer.data).to.include('END PGP MESSAGE');
                    context.decrypt(answer.data).then(function (result) {
                        expect(result).to.not.be.empty;
                        expect(result.data).to.be.a('string');
                        expect(result.data).to.equal(inputvalues.encrypt.good.data);
                        context.connection.disconnect();
                        done();
                    });
                });
        });
    });
    it('Roundtrip does not destroy trailing whitespace',
        function (done) {
            let prm = Gpgmejs.init();
            prm.then(function (context) {
                let data = 'Keks. \rKeks \n Keks \r\n';
                context.encrypt(data,
                    inputvalues.encrypt.good.fingerprint).then(
                    function (answer) {
                        expect(answer).to.not.be.empty;
                        expect(answer.data).to.be.a("string");
                        expect(answer.data).to.include(
                            'BEGIN PGP MESSAGE');
                        expect(answer.data).to.include(
                            'END PGP MESSAGE');
                        context.decrypt(answer.data).then(
                            function (result) {
                                expect(result).to.not.be.empty;
                                expect(result.data).to.be.a('string');
                                expect(result.data).to.equal(data);
                                context.connection.disconnect();
                                done();

                            });
                    });
            });
        }).timeout(5000);

    for (let j = 0; j < inputvalues.encrypt.good.data_nonascii_32.length; j++){
        it('Roundtrip with >1MB non-ascii input meeting default chunksize (' + (j + 1) + '/' + inputvalues.encrypt.good.data_nonascii_32.length + ')',
            function (done) {
                let input = inputvalues.encrypt.good.data_nonascii_32[j];
                expect(input).to.have.length(32);
                let prm = Gpgmejs.init();
                prm.then(function (context) {
                    let data = '';
                    for (let i=0; i < 34 * 1024; i++){
                        data += input;
                    }
                    context.encrypt(data,
                        inputvalues.encrypt.good.fingerprint).then(
                        function (answer) {
                            expect(answer).to.not.be.empty;
                            expect(answer.data).to.be.a("string");
                            expect(answer.data).to.include(
                                'BEGIN PGP MESSAGE');
                            expect(answer.data).to.include(
                                'END PGP MESSAGE');
                            context.decrypt(answer.data).then(
                                function (result) {
                                    expect(result).to.not.be.empty;
                                    expect(result.data).to.be.a('string');
                                    expect(result.data).to.equal(data);
                                    context.connection.disconnect();
                                    done();
                                });
                        });
                });
        }).timeout(5000);
    };

    it('Encrypt-decrypt simple non-ascii', function (done) {
        //FAILS TODO: Check newline at the end
        let prm = Gpgmejs.init();
        prm.then(function (context) {
            data = encryptedData;
            context.decrypt(data).then(
                function (result) {
                    expect(result).to.not.be.empty;
                    expect(result.data).to.be.a('string');
                    expect(result.data).to.equal(inputvalues.encrypt.good.data_nonascii);
                    context.encrypt(inputvalues.encrypt.good.data_nonascii, inputvalues.encrypt.good.fingerprint).then(
                        function(result){
                            context.decrypt(result.data).then(function(answer){
                                expect(answer.data).to.equal('¡Äußerste µ€ før ñoquis@hóme! Добрый день');
                                context.connection.disconnect();
                                done();
                            });
                        });
                    });

        });
    }).timeout(6000);

});
