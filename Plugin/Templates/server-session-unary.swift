// {{ method.name }} (Unary)
public class {{ .|session:protoFile,service,method }} : {{ .|service:protoFile,service }}Session {
  private var provider : {{ .|provider:protoFile,service }}

  /// Create a session.
  fileprivate init(handler:gRPC.Handler, provider: {{ .|provider:protoFile,service }}) {
    self.provider = provider
    super.init(handler:handler)
  }

  /// Run the session. Internal.
  fileprivate func run(queue:DispatchQueue) throws {
    try handler.receiveMessage(initialMetadata:initialMetadata) {(requestData) in
      if let requestData = requestData {
        let requestMessage = try {{ method|input }}(protobuf:requestData)
        let replyMessage = try self.provider.{{ method.name|lowercase }}(request:requestMessage, session: self)
        try self.handler.sendResponse(message:replyMessage.serializeProtobuf(),
                                      statusCode:self.statusCode,
                                      statusMessage:self.statusMessage,
                                      trailingMetadata:self.trailingMetadata)
      }
    }
  }
}
