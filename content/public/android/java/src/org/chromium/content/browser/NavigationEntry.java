// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.graphics.Bitmap;

/**
 * Represents one entry in the navigation history of a page.
 */
public class NavigationEntry {

    private String mUrl;
    private String mOriginalUrl;
    private String mVirtualUrl;
    private String mTitle;
    private Bitmap mFavicon;

    /**
     * Default constructor.
     */
    /* package */ NavigationEntry(String url, String virtualUrl, String originalUrl, String title,
            Bitmap favicon) {
        mUrl = url;
        mVirtualUrl = virtualUrl;
        mOriginalUrl = originalUrl;
        mTitle = title;
        mFavicon = favicon;
    }

    /**
     * @return The actual URL of the page. For some about pages, this may be a
     *         scary data: URL or something like that. Use GetVirtualURL() for
     *         showing to the user.
     */
    public String getUrl() {
        return mUrl;
    }

    /**
     * @return The virtual URL, when nonempty, will override the actual URL of
     *         the page when we display it to the user. This allows us to have
     *         nice and friendly URLs that the user sees for things like about:
     *         URLs, but actually feed the renderer a data URL that results in
     *         the content loading.
     *         <p/>
     *         GetVirtualURL() will return the URL to display to the user in all
     *         cases, so if there is no overridden display URL, it will return
     *         the actual one.
     */
    public String getVirtualUrl() {
        return mVirtualUrl;
    }

    /**
     * @return The URL that caused this NavigationEntry to be created.
     */
    public String getOriginalUrl() {
        return mOriginalUrl;
    }

    /**
     * @return The title as set by the page. This will be empty if there is no
     *         title set. The caller is responsible for detecting when there is
     *         no title and displaying the appropriate "Untitled" label if this
     *         is being displayed to the user.
     */
    public String getTitle() {
        return mTitle;
    }

    /**
     * @return The favicon of the page. This may be null.
     */
    public Bitmap getFavicon() {
        return mFavicon;
    }

}
