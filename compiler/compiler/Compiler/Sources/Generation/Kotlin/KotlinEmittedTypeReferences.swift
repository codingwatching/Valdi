//
//  File.swift
//  
//
//  Created by Simon Corsin on 2/17/23.
//

import Foundation

class KotlinEmittedTypeReferences: EmittedIdentifiers, CodeWriterContent {

    // Per-index usage: an index used anywhere other than a deferrable function return position is
    // "eager" and must always be walked. An index used in a deferrable return position is a candidate.
    // The lazy set is candidates minus eager: references reachable ONLY through deferred returns.
    private var eagerIndices = Set<Int>()
    private var deferrableReturnIndices = Set<Int>()

    func recordUsage(index: Int, inDeferrableReturnType: Bool) {
        if inDeferrableReturnType {
            deferrableReturnIndices.insert(index)
        } else {
            eagerIndices.insert(index)
        }
    }

    var lazyReturnIndices: [Int] {
        return deferrableReturnIndices.subtracting(eagerIndices).sorted()
    }

    var initializationString: CodeWriter {
        let body = identifiers.map {
             "\($0)::class"
        }.joined(separator: ", ")
        let out = CodeWriter()
        out.appendBody("[")
        out.appendBody(body)
        out.appendBody("]")
        return out
    }

    var content: String {
        return initializationString.content
    }

}

/// Renders the `lazyReturnTypeReferences` annotation value (an IntArray literal) from a
/// [KotlinEmittedTypeReferences], evaluated lazily at code-render time (after the schema is written).
/// Renders empty when there are no lazy-only return references so the annotation param is omitted.
class KotlinLazyReturnTypeReferences: CodeWriterContent {

    private let typeReferences: KotlinEmittedTypeReferences

    init(_ typeReferences: KotlinEmittedTypeReferences) {
        self.typeReferences = typeReferences
    }

    var content: String {
        let indices = typeReferences.lazyReturnIndices
        if indices.isEmpty {
            return ""
        }
        return "[" + indices.map { String($0) }.joined(separator: ", ") + "]"
    }

}
