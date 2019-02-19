[@bs.module "urql"] external provider: ReasonReact.reactClass = "Provider";

let component = ReasonReact.statelessComponent("Provider");

let make = (~client: UrqlClient.client, children) =>
  ReasonReact.wrapJsForReason(
    ~reactClass=provider,
    ~props={"client": client},
    children,
  );
