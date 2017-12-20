// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.payments;

import android.test.suitebuilder.annotation.MediumTest;

import org.chromium.base.test.util.Feature;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.autofill.AutofillTestHelper;
import org.chromium.chrome.browser.autofill.PersonalDataManager.CreditCard;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;

/**
 * A payment integration test for checking whether user can make a payment via a credit card. This
 * user has a valid  credit card without a billing address on file.
 */
public class PaymentRequestCcCanMakePaymentQueryTest extends PaymentRequestTestBase {
    public PaymentRequestCcCanMakePaymentQueryTest() {
        super("payment_request_can_make_payment_query_cc_test.html");
    }

    @Override
    public void onMainActivityStarted() throws InterruptedException, ExecutionException,
            TimeoutException {
        // The user has a valid credit card without a billing address on file. This is sufficient
        // for canMakePayment() to return true.
        new AutofillTestHelper().setCreditCard(new CreditCard("", "https://example.com", true, true,
                "Jon Doe", "4111111111111111", "1111", "12", "2050", "visa", R.drawable.pr_visa,
                "" /* billingAddressId */, "" /* serverId */));
    }

    @MediumTest
    @Feature({"Payments"})
    public void testCanMakePayment() throws InterruptedException, ExecutionException,
            TimeoutException {
        triggerUIAndWait(mCanMakePaymentQueryResponded);
        expectResultContains(new String[]{"true"});

        // Repeating a query does not count against the quota.
        clickNodeAndWait("buy", mCanMakePaymentQueryResponded);
        expectResultContains(new String[]{"true"});

        clickNodeAndWait("buy", mCanMakePaymentQueryResponded);
        expectResultContains(new String[]{"true"});

        // Different queries are throttled for a period of time.
        clickNodeAndWait("other-buy", mCanMakePaymentQueryResponded);
        expectResultContains(new String[]{"Query quota exceeded"});

        // Repeating the same query again does not count against the quota.
        clickNodeAndWait("buy", mCanMakePaymentQueryResponded);
        expectResultContains(new String[]{"true"});
    }
}
