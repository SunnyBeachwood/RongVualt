package com.sovworks.eds.android.providers;

import android.net.Uri;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * In-memory token registry for active DocumentsProvider sessions. Document IDs
 * intentionally contain no source URI, host path, volume label, or inner path.
 * A provider process restart invalidates its tokens, matching the fact that the
 * underlying unlocked session must be reopened after process death.
 */
final class DocumentIdRegistry
{
    private static final DocumentIdRegistry INSTANCE = new DocumentIdRegistry();

    static DocumentIdRegistry getInstance()
    {
        return INSTANCE;
    }

    synchronized String register(Uri locationUri)
    {
        String locationKey = locationUri.toString();
        String existing = _documentIdByLocation.get(locationKey);
        if(existing != null)
            return existing;

        String documentId = UUID.randomUUID().toString();
        _locationByDocumentId.put(documentId, locationUri);
        _documentIdByLocation.put(locationKey, documentId);
        return documentId;
    }

    synchronized Uri resolve(String documentId)
    {
        Uri locationUri = _locationByDocumentId.get(documentId);
        if(locationUri == null)
            throw new IllegalArgumentException("Unknown or expired document id");
        return locationUri;
    }

    synchronized void clear()
    {
        _locationByDocumentId.clear();
        _documentIdByLocation.clear();
    }

    private final Map<String, Uri> _locationByDocumentId = new HashMap<>();
    private final Map<String, String> _documentIdByLocation = new HashMap<>();
}
